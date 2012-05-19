/* tqw.c */

#include <sys/times.h>
#include <dirent.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <pthread.h>

#include <sys/types.h>
#include <gtk/gtk.h>

#include "tqw.h"

#define SLATE 0x00635D4A
#define MAXCOLS 256

typedef struct {
    float x, y, z;
} Point3D;

typedef struct {
    Point3D pts[4];
} BlipLoc;

static BlipLoc bliplocs[MAXCOLS * 8];
static UChar blips[ARSIZE(bliplocs)];

#define M_PI 3.14159265358979323846

typedef struct {
    GtkWidget *window, *board;
    vfunc col_routine, rot_routine;
    bool mapped, changed;
    int sboffs, intr_col_idx, nblips;
    UChar latch;
} Config;

static Config cfg;

#define BWIDTH 200
#define BHEIGHT 200
#define VDIST 100

/* -------------------- */

static void destroy(GtkWidget *widget, gpointer data) { exit(0); }

void Panic(const char *msg, ...)
{
    va_list ap;

    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    va_end(ap);
    fputc('\n', stderr);

    exit(1);
}

static GdkColor colors[8];

#define CV16(b) (b ? 0xFFFF : 0)

static void build_cube()
{
    int i, r, g, b;
    gboolean *success;

    for (i = r = 0; r < 2; r++) {
        for (g = 0; g < 2; g++) {
            for (b = 0; b < 2; b++, i++) {
                colors[i].red = CV16(r);
                colors[i].green = CV16(g);
                colors[i].blue = CV16(b);
            }
        }
    }

    success = alloca(ARSIZE(colors) * sizeof(gboolean));
    if (gdk_colormap_alloc_colors(gdk_colormap_get_system(), colors,
                                  ARSIZE(colors), FALSE, TRUE, success) != 0) {
        Panic("Could not allocate color cube.");
    }
}

/* NOTE: not thread-safe. */
static GdkColor *findColor(int rgb)
{
    static GdkColor ans;

    memset(&ans, 0, sizeof(ans));
    ans.red =   (rgb >> 16) & 0xFF; ans.red   |= ans.red   << 8;
    ans.green = (rgb >> 8) & 0xFF;  ans.green |= ans.green << 8;
    ans.blue =   rgb & 0xFF;        ans.blue  |= ans.blue  << 8;

    gdk_colormap_alloc_color(gdk_colormap_get_system(), &ans, FALSE, TRUE);

    return &ans;
}

/* ---- */

static GtkItemFactoryEntry menu_items[] = {
    { "/Quit", "<control>Q", exit },
    { "/Next", "<control>N", changeDisplay }
};

static GtkWidget *buildMenus(GtkWidget *window)
{
    GtkItemFactory *item_factory;
    GtkAccelGroup *accel_group;

    accel_group = gtk_accel_group_new ();
    item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", 
                                        accel_group);
    gtk_item_factory_create_items(item_factory, ARSIZE(menu_items),
                                  menu_items, 0);

    gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

    return gtk_item_factory_get_widget(item_factory, "<main>");
}

/* ---------- blips ---------- */

static void init_bliplocs()
{
    int i, j, k;
    float ang = (2 * M_PI / (cfg.nblips / 8));
    float x1, z1, x2, z2;
    BlipLoc *bl;

    x1 = 60, z1 = 0;
    for (i = k = 0, bl = bliplocs;
         k < cfg.nblips; x1 = x2, z1 = z2, i++) {
        x2 = 60 * cos((i + 1) * ang);
        z2 = 60 * sin((i + 1) * ang);
        for (j = 7; j >= 0; j--, k++, bl++) {
            bl->pts[0].x = x1;
            bl->pts[0].y = j * 3;
            bl->pts[0].z = z1;

            bl->pts[1].x = x2;
            bl->pts[1].y = j * 3;
            bl->pts[1].z = z2;

            bl->pts[2].x = x2;
            bl->pts[2].y = (j + 1) * 3;
            bl->pts[2].z = z2;

            bl->pts[3].x = x1;
            bl->pts[3].y = (j + 1) * 3;
            bl->pts[3].z = z1;
        }
    }
}

static void init_blips(void)
{
    int i;

    for (i = 0; i < cfg.nblips; i++) {
        blips[i] = 0;
    }
}

/* ---- drawin utility ---- */

static void project(const Point3D *from, GdkPoint *to,
                    int offsx, int offsy)
{
    float x, y, z;

    z = VDIST / (from->z + VDIST);
    x = from->x * z;
    y = -(BHEIGHT/4 + from->y) * z;
    to->x = x + offsx + (BWIDTH / 2) + 0.5;
    to->y = y + BHEIGHT + offsy + 0.5;
}

static void draw_blip(GdkPixmap *osb, GdkGC *gc,
                      UChar blip, BlipLoc *bp, int offsx, int offsy)
{
    GdkPoint *pts;
    int i, npts;

    if (blip) {
        npts = sizeof(bp->pts)/sizeof(bp->pts[0]);
        pts = (GdkPoint *) alloca(sizeof(GdkPoint) * npts);

        for (i = 0; i < npts; i++)
            project(&bp->pts[i], &pts[i], offsx, offsy);

        gdk_gc_set_foreground(gc, &colors[blip]);
        gdk_draw_polygon(osb, gc, TRUE, pts, npts);
    }
}

static void board_update(GdkRectangle *clip)
{
    GtkWidget *board = cfg.board;
    GdkGC *g = board->style->fg_gc[board->state];
    GdkPixmap *osb;
    int x, y, width, height, depth, i, bi;
    GdkGCValues valz;

    gdk_gc_get_values(g, &valz);
    gdk_window_get_geometry(cfg.window->window,
                            &x, &y, &width, &height, &depth);
    osb = gdk_pixmap_new(cfg.window->window,
                         clip->width, clip->height, depth);
    gdk_gc_set_foreground(g, findColor(SLATE));
    gdk_draw_rectangle(osb, g, TRUE, 0, 0, clip->width, clip->height);

    for (i = 0, bi = 8 * cfg.sboffs; i < cfg.nblips; i++, bi++) {
        while (bi < 0) bi += cfg.nblips;
        while (bi > cfg.nblips) bi -= cfg.nblips;
        draw_blip(osb, g, blips[i], &bliplocs[bi], -clip->x, -clip->y);
    }

    gdk_draw_pixmap(board->window, g, osb, 0, 0, clip->x, clip->y,
                    clip->width, clip->height);
    gdk_gc_set_foreground(g, &valz.foreground);

    gdk_pixmap_unref(osb);
}

static gboolean board_expose(GtkWidget *board,
                             GdkEventExpose *evt, gpointer *ud)
{
    board_update(&evt->area);        
    return TRUE;
}

/* ---------------------------------------- */

static void redraw(void)
{
    GdkRectangle rect;

    rect.x = rect.y = 0;
    rect.width = BWIDTH;
    rect.height = BHEIGHT;
    board_update(&rect);
    cfg.changed = false;
}

static void sb_val_changed(GtkAdjustment *adj, gpointer ud)
{
    cfg.sboffs = -adj->value;
    redraw();
}

/* ---------------------------------------- */

static gboolean interrupt(gpointer ud)
{
    cfg.intr_col_idx++;
    if (cfg.intr_col_idx >= cfg.nblips / 8) {
        if (cfg.rot_routine) cfg.rot_routine();
        cfg.intr_col_idx = 0;
        if (cfg.changed && cfg.mapped) redraw();
    }
    if (cfg.col_routine) cfg.col_routine();

    return TRUE; /* reschedule */
}

/* ---------------------------------------- */

/* ugh.  can't draw on the board till it gets mapped. */
static void board_map(GtkWidget *board, gpointer *ud)
{
    cfg.mapped = true;
}

static void buildTable(void)
{
    GtkWidget *vbox, *mbar, *sbar;
    GtkObject *adj;

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(cfg.window), vbox);
    gtk_widget_show(vbox);

    mbar = buildMenus(cfg.window);
    gtk_box_pack_start(GTK_BOX(vbox), mbar, FALSE, FALSE, 0);
    gtk_widget_show(mbar);

    cfg.board = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(vbox), cfg.board, TRUE, TRUE, 0);
    gtk_signal_connect(GTK_OBJECT(cfg.board), "map",
                       GTK_SIGNAL_FUNC(board_map), 0);
    gtk_signal_connect(GTK_OBJECT(cfg.board), "expose_event",
                       GTK_SIGNAL_FUNC(board_expose), 0);
    gtk_drawing_area_size(GTK_DRAWING_AREA(cfg.board), BWIDTH, BHEIGHT);
    gtk_widget_show(cfg.board);

    adj = gtk_adjustment_new(0, -(MAXCOLS / 2), +(MAXCOLS / 2), 1, 10, 10);
    gtk_signal_connect(adj, "value_changed",
                       GTK_SIGNAL_FUNC(sb_val_changed), 0);
    sbar = gtk_hscrollbar_new(GTK_ADJUSTMENT(adj));
    gtk_box_pack_start(GTK_BOX(vbox), sbar, FALSE, FALSE, 0);
    gtk_widget_show(sbar);
    
    cfg.intr_col_idx = 0xFFFF; /* force rot interrupt first time */
    g_timeout_add(1, interrupt, 0);
}

/* --------------- */

void TQWSetRotationInterrupt(vfunc f)
{
    cfg.rot_routine = f;
}

void TQWSetColumnInterrupt(vfunc f)
{
    cfg.col_routine = f;
}

void TQWLoadLatch(UChar c)
{
    cfg.latch = c;
}

static void latch_data(UChar mask)
{
    int r, ci, mm;

    for (mm = 1, r = 0, ci = cfg.intr_col_idx * 8;
         r < 8; r++, ci++, mm <<= 1) {
        if (cfg.latch & mm) blips[ci] |= mask;
        else blips[ci] &= ~mask;
    }

    cfg.changed = true;
}

void TQWStrobeRed(void) { latch_data(0x04); }
void TQWStrobeGreen(void) { latch_data(0x02); }
void TQWStrobeBlue(void) { latch_data(0x01); }

#define PROBE 2567

UShort TQWGetAndResetRotationInterruptTime(void)
{
    return PROBE;
}

void TQWSetColumnInterruptTimer(UChar t)
{
    int nb, ncols;

    ncols = PROBE / t;
    if (ncols < 8 || ncols > 256)
        Panic("Requested %d columns -- I only support ncols in [8..256]");

    nb = 8 * ncols;

    if (cfg.nblips != nb) {
        cfg.nblips = nb;
        init_bliplocs();
        init_blips();
    }
}

/* -------------------- */

static void *runSim(void *data)
{
    memset(&cfg, 0, sizeof(cfg));

    gtk_init(0, 0);
    gdk_rgb_init();
    gtk_widget_set_default_colormap(gdk_rgb_get_cmap());
    gtk_widget_set_default_visual(gdk_rgb_get_visual());

    cfg.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_signal_connect(GTK_OBJECT(cfg.window), "destroy",
                       GTK_SIGNAL_FUNC(destroy), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(cfg.window), 0);

    buildTable();
    build_cube();
    gtk_widget_show(cfg.window);
    gtk_main();

    return 0;
}

void TQWInit(void)
{
    pthread_t pt;

    pthread_create(&pt, 0, runSim, 0);
}


/* The following are only for the simulator */

void TQWDelay(UChar d)
{
    ULong usec;
    struct timespec ts_req;

    usec = d * 4762;
    ts_req.tv_sec = usec / (1000 * 1000) ;
    usec -= (1000 * 1000 * ts_req.tv_sec);
    ts_req.tv_nsec = usec * 1000;
    nanosleep(&ts_req, 0);
}

#include <yirl/entity.h>
#include <yirl/events.h>
#include <yirl/entity-script.h>
#include <yirl/game.h>
#include <yirl/canvas.h>

static const char *sc_path = "falling_guy.score";

static int old_tl;
static int r_w = 40;
static int r_h = 10;

static int w_screen = 800;
static int h_screen = 600;

static const char *m_path;

void *fg_action(int nbArgs, void **args)
{
	Entity *fg = args[0];
	Entity *eves = args[1];
	Entity *g = yeGet(fg, "guy");
	Entity *wind = yeGet(fg, "wind");
	Entity *r_list = yeGet(fg, "r_list");
	int dir_change = 0;
	int y_change = 0;
	static int xdir;
	static int ydir;
	int s;
	int dificulty = 10;
	Entity *gp;
	Entity *gs;
	YE_NEW(String, score_str, "score: ");

	if (!g) {
		yePushBack(fg, ywCanvasNewImgByPath(fg, w_screen / 2 - 16, 5,
						    "./super_guy.png"), "guy");
		g = yeGet(fg, "guy");
	}
	printf("%d\n", ygGetInt(sc_path));
	ygIntAdd(sc_path, 11 - dificulty);
	s = ygGetInt(sc_path);
	yeStringAddInt(score_str, s);
	ywCanvasStringSet(yeGet(fg, "score"), score_str);
	ywCanvasStringSet(wind, yeGet(yeGet(fg, "winds"), ywTurnId % 3));

	if (s > 1200)
		dificulty = 1;
	else if (s > 1000)
		dificulty = 2;
	else if (s > 900)
		dificulty = 3;
	else if (s > 800)
		dificulty = 4;
	else if (s > 700)
		dificulty = 5;
	else if (s > 500)
		dificulty = 6;
	else if (s > 400)
		dificulty = 7;
	else if (s > 200)
		dificulty = 8;
	else if (s > 100)
		dificulty = 9;

	if (yevIsKeyDown(eves, Y_LEFT_KEY)) {
		xdir = -1;
		if (xdir > -1)
			dir_change = 1;
	} else if (yevIsKeyDown(eves, Y_RIGHT_KEY)) {
		xdir = 1;
		if (xdir < 1)
			dir_change = 1;
	}
	if (yevIsKeyDown(eves, Y_DOWN_KEY)) {
		ydir = 10;
		if (ydir < 1)
			y_change = 1;
	} else if (yevIsKeyDown(eves, Y_UP_KEY)) {
		ydir = -3;
		if (ydir > -1)
			y_change = 1;
	}

	if (!dir_change &&
	    yevCheckKeysInt(eves, YKEY_UP, (int []){Y_RIGHT_KEY, Y_LEFT_KEY})) {
		xdir = 0;
	}
	if (!y_change &&
	    yevCheckKeysInt(eves, YKEY_UP, (int []){Y_DOWN_KEY, Y_UP_KEY})) {
		ydir = 0;
	}
	ywCanvasMoveObjXY(g, 5 * xdir, ydir);
	gp = ywCanvasObjPos(g);
	gs = ywCanvasObjSize(NULL, g);
	if (ywPosX(gp) < 0 || ywPosX(gp) > w_screen - ywSizeW(gs))
		ywCanvasMoveObjXY(g, -(5 * xdir), 0);
	if (ywPosY(gp) < 0 || ywPosY(gp) > h_screen - ywSizeH(gs))
		ywCanvasMoveObjXY(g, 0, -ydir);
	if ((yuiRand() % dificulty) == 0) {
		YE_NEW(Array, r_info);

		ywSizeCreate(r_w, r_h, r_info, NULL);
		yeCreateString("rgba: 0 0 0 255", r_info, NULL);
		yePushBack(r_list,
			   ywCanvasNewRect(fg, yuiRand() % w_screen - r_w / 2,
					   h_screen, r_info),
			   NULL);
	}
	// we reget it se if we have move back because of
	// out of screen we're still at good place
	gp = ywCanvasObjPos(g);
	ywCanvasObjSetPos(wind, ywPosX(gp),
			  ywPosY(gp)  -
			  ywSizeH(ywCanvasObjSize(NULL, wind)));

	YE_FOREACH(r_list, r) {
		ywCanvasMoveObjXY(r, 0, -10);
		if (ywCanvasObjectsCheckColisions(g, r)) {
			printf("die because your DEAD!\n");
			ygTerminate();
		}
		if (ywCanvasObjPosY(r) < 0) {
			ywCanvasRemoveObj(fg, r);
			yeRemoveChild(r_list, r);
		}
	}
	return (void *)ACTION;
}

void *fg_kaboum(int nbArgs, void **args)
{
	ywTurnLengthOverwrite = old_tl;
}

void *fg_init(int nbArgs, void **args)
{
	Entity *fg = args[0];

	YEntityBlock {
		fg.background = "rgba: 255 255 255 255";
		fg.action = fg_action;
		fg.destroy = fg_kaboum;
		fg.r_list= [];
		fg.winds = [
			"| |\n | \n|  ",
			"  |\n  |\n  |",
			"|  \n|  \n  |",
			];
	}
	old_tl = ywTurnLengthOverwrite;
	ywTurnLengthOverwrite = 70000;

	printf("new fg wid\n");
	void *ret = ywidNewWidget(fg, "canvas");

	printf("%s%s\n", m_path, "super_guy.png");
	yePushBack(fg, ywCanvasNewText(fg, 100, 10,
				       yeGet(yeGet(fg, "winds"), 0)),
		   "wind");
	yePushBack(fg, ywCanvasNewTextByStr(fg, 0, 10, ""), "score");
	ygReCreateInt(sc_path, 0);
	return ret;
}

void *mod_init(int nbArg, void **args)
{
	Entity *mod = args[0];
	Entity *init;

	printf("FG init\n");
	init = yeCreateArray(NULL, NULL);
	YEntityBlock {
		init.name = "falling_guy";
		init.callback = fg_init;
		mod.name = "falling_guy";
		mod.test_fg = [];
		mod.test_fg["<type>"] = "falling_guy";
		mod.starting_widget = "test_fg";
		mod["window size"] = [800, 600];
		mod["window name"] = "Falling Guy";
	}
	m_path = yeGetStringAt(mod, "$path");
	ywidAddSubType(init);
	yuiRandInit();
	return mod;
}

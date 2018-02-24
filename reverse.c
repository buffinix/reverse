//1752894 张子健 tj se
#include <stdio.h>
#include<string.h>

#define SPACE 0
#define BLACK 1
#define WHITE 2
#define ACTUAL 10
#define IMAGINE 11
#define MAX_DEPTH 4
#define MMAX 20000
#define MMIN -20000
#define READ 20
#define RECORD 30

const int dir[8][2] = { { -1, 0 },{ -1, -1 },{ 0, -1 },{ 1, -1 },{ 1, 0 },{ 1, 1 },{ 0, 1 },{ -1, 1 } };//方向的转动 
int chessmap[MAX_DEPTH + 1][16][16];
typedef struct {
	int x;
	int y;
	int side;
	int score;
} placement;
placement bestmove;
//由于使用了memcpy来替代un_makemove 所以棋盘和movelist都需要多层
placement movelist[MAX_DEPTH + 1][80];//存放由RECORD模式下每层由find_move找出的走子方案
int move_num;
int my_side;
int other_side;
int step_count;
int value[16][16] = {
	{ 200,-8,10,5,5,5,5,5,5,5,5,5,5,10,-8,200 } ,
	{ -8,-100,1,1,1,1,1,1,1,1,1,1,1,1,-100,-8 }  ,
	{ 10,1,3,2,2,2,2,2,2,2,2,2,2,3,1,10 } ,
	{ 5,1,2,1,1,1,1,1,1,1,1,1,1,2,1,5 } ,
	{ 5,1,2,1,1,1,1,1,1,1,1,1,1,2,1,5 } ,
	{ 5,1,2,1,1,1,1,1,1,1,1,1,1,2,1,5 } ,
	{ 5,1,2,1,1,1,1,1,1,1,1,1,1,2,1,5 } ,
	{ 5,1,2,1,1,1,1,1,1,1,1,1,1,2,1,5 } ,
	{ 5,1,2,1,1,1,1,1,1,1,1,1,1,2,1,5 } ,
	{ 5,1,2,1,1,1,1,1,1,1,1,1,1,2,1,5 } ,
	{ 5,1,2,1,1,1,1,1,1,1,1,1,1,2,1,5 } ,
	{ 5,1,2,1,1,1,1,1,1,1,1,1,1,2,1,5 } ,
	{ 5,1,2,1,1,1,1,1,1,1,1,1,1,2,1,5 } ,
	{ 10,1,3,2,2,2,2,2,2,2,2,2,2,3,1,10 },
	{ -8,-100,1,1,1,1,1,1,1,1,1,1,1,1,-100,-8 },
	{ 200,-8,10,5,5,5,5,5,5,5,5,5,5,10,-8,200 },
};

void initialize(void);
_Bool judge_8_dir(int x, int y, int side, int depth);//判断一处是否可以落子

_Bool judge_1_dir(int x, int y, int dir, int side, int depth);//判断一个方向是否可翻转

void make_move(int type, int depth, int side, int number);//type分为 ACYTUAL和 IMAGINE
//分别对应于“真正”落子和“搜索”中的落子 并告知turnover函数落子位置

int alphabeta(int depth, int alpha, int beta);
int max(int a, int b);
int min(int a, int b);
int find_move(int choice, int depth, int side);//choice分 READ和RECORD 
//分别对应仅仅“计算”可落子的个数 和 把可落子位置“记录”下来 调用judge_8_dir判断是否可以落子

void add_list(int depth, int side, int x, int y);//配合find_move 记录位置 

int evaluate(int depth);//选用估值表和凝聚手以及前期的行动力

void submit(void);
void cleanup(void);//为防止上步干扰下步 
void turnover(int depth, int side, int x, int y);//被make_move调用 并8次调用judge_1_dir并进行反转

_Bool gameover(int depth);
_Bool cohesion(int x, int y, int depth);//判断给定位置是否为凝聚手（相当于同学们所说潜在行动力）
//周围有3个及以上空白的会被判定为不凝聚的 引起扣分

int main(void)
{
	char commond[5];
	int j;
	step_count = -1;
	scanf("%s", commond);
	while (commond[0] != 'E')
	{
		step_count++;
		switch (commond[0])
		{
		case('S'):
			initialize();
			break;
		case('P'):
			make_move(ACTUAL, MAX_DEPTH, other_side, 0);
			alphabeta(MAX_DEPTH, MMIN, MMAX);
			break;
		case('T'):
			submit();
			make_move(ACTUAL, MAX_DEPTH, my_side, 0);
			cleanup();
			j = find_move(READ, MAX_DEPTH, other_side);
			if (j == 0)//若对方不能走子，我再想一个走子 
			{
				alphabeta(MAX_DEPTH, MMIN, MMAX);
				cleanup();
			}
			else
			{
				move_num = 0;
				cleanup();
				break;
			}
		}
		scanf("%s", commond);
	}
	return 0;
}

void initialize(void)
{
	chessmap[MAX_DEPTH][8][7] = chessmap[MAX_DEPTH][7][8] = BLACK;
	chessmap[MAX_DEPTH][7][7] = chessmap[MAX_DEPTH][8][8] = WHITE;//放置初始子
	scanf(" %d", &my_side);
	if (my_side == BLACK)
	{
		other_side = WHITE;//避免第一步搜索 
		bestmove.x = 8;
		bestmove.y = 9;
		bestmove.side = BLACK;
	}
	else
	{
		other_side = BLACK;
	}
	bestmove.score = MMIN;
	printf("OK\n");
	fflush(stdout);
}
int max(int a, int b)
{
	if (a > b)
		return a;
	else
		return b;
}
int min(int a, int b)
{
	if (a < b)
		return a;
	else
		return b;
}

int alphabeta(int depth, int alpha, int beta)
{
	int side;
	int count;
	int i;

	if (depth == 0 || gameover(depth))
		return evaluate(depth);

	if ((MAX_DEPTH - depth) % 2 == 0)
		side = my_side;
	else
		side = other_side;

	if (side == my_side)//对应我方搜索，回复alpha。 
	{
		count = find_move(RECORD, depth, side);//寻找当前深度棋盘的走子方案，返回走子方案个数
		for (i = 0; i < count; i++)
		{
			make_move(IMAGINE, depth, side, i);//按顺序依次走子，走子前会memcpy上层棋盘到本层
			alpha = max(alpha, alphabeta(depth - 1, alpha, beta));
			if (depth == MAX_DEPTH)//靠近根节点处保留走子 
			{
				movelist[MAX_DEPTH][i].score = alpha;
				if (movelist[MAX_DEPTH][i].score > bestmove.score)
				{
					bestmove = movelist[MAX_DEPTH][i];
				}
			}
			if (beta <= alpha)
				break;
		}
		return alpha;
	}
	else
	{
		count = find_move(RECORD, depth, side);
		for (i = 0; i < count; i++)
		{
			make_move(IMAGINE, depth, side, i);
			beta = min(beta, alphabeta(depth - 1, alpha, beta));
			if (beta <= alpha)
				break;
		}
		return beta;
	}
}
void cleanup(void)
{
	bestmove.score = -50000;
	memset(movelist, 0, sizeof(movelist));
}
int evaluate(int depth)
{
	int i, j;
	int count_my = 0, count_other = 0;
	_Bool my_lose = 1;
	_Bool other_lose = 1;

	for (i = 0; i < 16; i++)//在遍历棋盘中 有三种情况 我方 、对方、 空白 
		//分别用于对我方积分 对方积分 以及行动力的评估
	{
		for (j = 0; j < 16; j++)
		{
			if (chessmap[depth][i][j] == my_side)
			{
				my_lose = 0;//判断是否已经出现输赢 
				count_my += value[i][j];//价目表的加分 
				if (cohesion(i, j, depth))
					count_my -= 3;//对不凝聚手的扣分 

			}
			else if (chessmap[depth][i][j] == other_side)
			{
				other_lose = 0;
				count_other += value[i][j];
				if (cohesion(i, j, depth))
					count_other -= 3;
			}
			/*else if(step_count<80)//前80积分行动力
			{
				if (judge_8_dir(i, j, my_side, depth))
					count_my += 3;
				if (judge_8_dir(i, j, other_side, depth))
					count_other += 3;
			}*/
		}
	}
	if (my_lose)
		return -20000;
	if (other_lose)
		return 20000;
	return count_my - count_other;
}
_Bool cohesion(int x, int y, int depth)//判断给定位置的子周围是否有三个及以上的空位 是则返回1 否则0
{
	int dir_chos, row_dir, col_dir;
	int count = 0;
	for (dir_chos = 0; dir_chos < 8; dir_chos++)
	{
		row_dir = dir[dir_chos][0];
		col_dir = dir[dir_chos][1];
		if (y + col_dir > -1 && y + col_dir < 16 &&
			x + row_dir > -1 && x + row_dir < 16)
		{
			if (chessmap[depth][x + row_dir][y + col_dir] == SPACE)
			{
				return 1;
			}
		}
	}
	return 0;
}
void submit(void)
{
	printf("%d %d\n", bestmove.x, bestmove.y);
	fflush(stdout);
}
void add_list(int depth, int side, int x, int y)//对于find_move中选择为RECORD的情况 记录走子
{
	movelist[depth][move_num].x = x;
	movelist[depth][move_num].y = y;
	movelist[depth][move_num].side = side;
	move_num++;
}

void make_move(int type, int depth, int side, int number)
{
	int x;
	int y;

	if (type == ACTUAL)//如果是真下（turn和place对应的），再分别敌我
	{
		if (side == my_side)//我方从best move走子
		{
			x = bestmove.x;
			y = bestmove.y;
			side = bestmove.side;
			turnover(MAX_DEPTH, side, x, y);
		}
		else
		{
			scanf(" %d %d", &x, &y);//对方接受
			turnover(MAX_DEPTH, side, x, y);
		}
	}
	else//搜索中的落子
	{
		memcpy(chessmap[depth - 1], chessmap[depth], sizeof(chessmap[depth]));//相当于un_make_move，恢复棋盘
		x = movelist[depth][number].x;
		y = movelist[depth][number].y;
		turnover(depth - 1, side, x, y);//落子
	}
}
int find_move(int choice, int depth, int side)//寻找可落子的个数（记录） 返回个数
{
	move_num = 0;
	int mobility = 0;
	int i, j;
	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 16; j++)
		{
			if (chessmap[depth][i][j] == SPACE&&judge_8_dir(i, j, side, depth))//是空白且8方向中至少有一个
			{
				mobility++;
				if (choice == RECORD)
					add_list(depth, side, i, j);
				continue;
			}
		}
	}
	return mobility;
}
_Bool judge_1_dir(int x, int y, int dir_cho, int side, int depth)//对于给定的方向 判定是否可以反转对方 若是返回1 否则0
{
	int opp_side;
	if (side == WHITE)
		opp_side = BLACK;
	else
		opp_side = WHITE;
	int cur_x = x + dir[dir_cho][0];
	int cur_y = y + dir[dir_cho][1];
	if (cur_x >= 0 && cur_x <= 15 && cur_y>=0 && cur_y<=15
		&& chessmap[depth][cur_x][cur_y] == opp_side)
	{
		while (chessmap[depth][cur_x][cur_y] == opp_side
			&& (cur_x + dir[dir_cho][0]) >= 0 && (cur_x + dir[dir_cho][0]) <= 15
			&& (cur_y + dir[dir_cho][1]) >= 0 && (cur_y + dir[dir_cho][1]) <= 15)
		{
			cur_x += dir[dir_cho][0];
			cur_y += dir[dir_cho][1];
		}
		return (chessmap[depth][cur_x][cur_y] == side) ? 1 : 0;
	}
	return 0;
}
_Bool judge_8_dir(int x, int y, int side, int depth)//对8个方向依次调用judge_1_dir 有一个可以的 则返回1 否则0
{
	int i;
	for (i = 0; i<8; i++)
	{
		if (judge_1_dir(x, y, i, side, depth))
			return 1;
	}
	return 0;
}
void turnover(int depth, int side, int x, int y)
{
	int dir_chos;
	int opp_side;

	if (side == WHITE)
		opp_side = BLACK;
	else
		opp_side = WHITE;
	chessmap[depth][x][y] = side;
	int cur_x;
	int cur_y;
	for (dir_chos = 0; dir_chos < 8; dir_chos++)
	{
		if (judge_1_dir(x, y, dir_chos, side, depth))//之所以分离judge_8_dir 和judge_1_dir 就是希望在此判断反转处使用 
			//从而精炼代码 避免重复
		{
			cur_x = x + dir[dir_chos][0];
			cur_y = y + dir[dir_chos][1];
			while (chessmap[depth][cur_x][cur_y] == opp_side
				&& (cur_x + dir[dir_chos][0]) >= 0 && (cur_x + dir[dir_chos][0]) <= 15
				&& (cur_y + dir[dir_chos][1]) >= 0 && (cur_y + dir[dir_chos][1]) <= 15)
			{
				chessmap[depth][cur_x][cur_y] = side;
				cur_x += dir[dir_chos][0];
				cur_y += dir[dir_chos][1];
			}
		}
	}
}
_Bool gameover(int depth)
{
	int i, j;
	_Bool flag = 1;
	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 16; j++)
		{
			if (chessmap[depth][i][j] == SPACE)
			{
				flag = 0;
			}
		}
	}
	return flag;
}

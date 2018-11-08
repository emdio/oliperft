/* OliPerft 1.0.1 - Bitboard Magic Move (c) Oliver Brausch 01.Jan.2008, ob112@web.de */
/* oliperft 6 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
Nodes: 8229523927 ms: 40610 knps: 202647 (VS2005 64bit AMD64 4600+)
Nodes: 8229523927 ms: 64860 knps: 126881 (VS2005 32bit AMD64 4600+)
Nodes: 8229523927 ms: 97251 knps: 84621 (gcc4 32bit AMD Opteron 1210HE)
*/
#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#ifdef _WIN32
#include <sys/timeb.h>
struct _timeb tv;
#else
#include <sys/time.h>
struct timeval tv;
struct timezone tz;
#endif

#define PAWN 1
#define KNIGHT 2
#define KING 3
#define ENP 4
#define BISHOP 5
#define ROOK 6
#define QUEEN 7

#define FROM(x) ((x) & 0x3F)
#define TO(x) (((x) >> 6) & 0x3F)
#define ONMV(x) (((x) >> 12) & 1)
#define PIECE(x) (((x) >> 13) & 7)  
#define PROM(x) (((x) >> 16) & 7)
#define CAP(x) (((x) >> 19) & 7)

#define _TO(x) ((x) << 6)
#define _ONMV(x) ((x) << 12)
#define _PIECE(x) ((x) << 13)
#define _PROM(x) ((x) << 16)
#define _CAP(x) ((x) << 19)
#define PREMOVE(f, p) ((f) | _ONMV(c) | _PIECE(p))

#define RATT1(f) rays[((f) << 7) | key000(board, f)]
#define RATT2(f) rays[((f) << 7) | key090(board, f) | 0x2000]
#define BATT3(f) rays[((f) << 7) | key045(board, f) | 0x4000]
#define BATT4(f) rays[((f) << 7) | key135(board, f) | 0x6000]
#define RXRAY1(f) rays[((f) << 7) | key000(board, f) | 0x8000]
#define RXRAY2(f) rays[((f) << 7) | key090(board, f) | 0xA000]
#define BXRAY3(f) rays[((f) << 7) | key045(board, f) | 0xC000]
#define BXRAY4(f) rays[((f) << 7) | key135(board, f) | 0xE000]

#define ROCC1(f) (RATT1(f) & board)
#define ROCC2(f) (RATT2(f) & board)
#define BOCC3(f) (BATT3(f) & board)
#define BOCC4(f) (BATT4(f) & board)
#define RMOVE1(f) (RATT1(f) & (~board))
#define RMOVE2(f) (RATT2(f) & (~board))
#define BMOVE3(f) (BATT3(f) & (~board))
#define BMOVE4(f) (BATT4(f) & (~board))
#define RCAP1(f, c) (RATT1(f) & colorb[(c)^1])
#define RCAP2(f, c) (RATT2(f) & colorb[(c)^1])
#define BCAP3(f, c) (BATT3(f) & colorb[(c)^1])
#define BCAP4(f, c) (BATT4(f) & colorb[(c)^1])
#define ROCC(f) (ROCC1(f) | ROCC2(f))
#define BOCC(f) (BOCC3(f) | BOCC4(f))
#define RMOVE(f) (RMOVE1(f) | RMOVE2(f))
#define BMOVE(f) (BMOVE3(f) | BMOVE4(f))
#define RCAP(f, c) (ROCC(f) & colorb[(c)^1])
#define BCAP(f, c) (BOCC(f) & colorb[(c)^1])

#define SHORTMOVE(x) ((x) & ((x)^board))
#define SHORTOCC(x) ((x) & board))
#define SHORTCAP(x, c) ((x) & colorb[(c)^1])

#define NMOVE(x) (SHORTMOVE(nmoves[x]))
#define KMOVE(x) (SHORTMOVE(kmoves[x]))
#define PMOVE(x, c) (pmoves[(x) | ((c)<<6)] & (~board))
#define NOCC(x) (SHORTOCC(nmoves[x]))
#define KOCC(x) (SHORTOCC(kmoves[x]))
#define POCC(x, c) (pcaps[(x) | ((c)<<6)] & board)
#define NCAP(x, c) (SHORTCAP(nmoves[x], (c)))
#define KCAP(x, c) (SHORTCAP(kmoves[x], (c)))
#define PCAP(x, c) (pcaps[(x) | ((c)<<6)] & colorb[(c)^1])
#define PCA3(x, c) (pcaps[(x) | ((c)<<6) | 128] & (colorb[(c)^1] | ((BIT[ENPASS]) & (c ? 0xFF0000LL : 0xFF0000000000LL))))
#define PCA4(x, c) (pcaps[(x) | ((c)<<6) | 256] & (colorb[(c)^1] | ((BIT[ENPASS]) & (c ? 0xFF0000LL : 0xFF0000000000LL))))

#define RANK(x, y) (((x) & 0x38) == (y))
#define TEST(f, b) (BIT[f] & (b))
#define ENPASS (flags & 63)
#define CASTLE (flags & 960)

typedef unsigned long long u64;
typedef unsigned long u32;
typedef int Move;

static u64 rays[0x10000];
u64 nmoves[64];
u64 kmoves[64];
u64 pmoves[128];
u64 pcaps[384];
int _knight[8] = {-17,-10,6,15,17,10,-6,-15};
int _king[8] = {-9,-1,7,8,9,1,-7,-8};
u64 pieceb[8];
u64 colorb[2];
u64 board;
u64 hashb;
static u64 BIT[64];
static u64 hashxor[0x10000];
static char LSB[0x10000];
static char BITC[0x10000] ;      
int flags;
int crevoke[64];
int onmove;
Move movelist[128*256];
int movenum[128];
int kingpos[2];

const char pieceChar[] = "*PNK.BRQ";

void setBit(int f, u64 *board) {
	*board |= BIT[f];
}

void xorBit(int f, u64 *board) {
	*board ^= BIT[f];
}

u64 getLowestBit(u64 bb) {
	return bb & (-(long long)bb);
}

void _parse_fen(char *fen) {
	char c, mv, pos[128], cas[4], enps[2];
	int i, j = 0, halfm = 0, fullm = 1, col = 0, row = 7;
	for (i = 0; i < 8; i++) {
		pieceb[i] = 0LL;
	}
	colorb[0] = colorb[1] = hashb = 0LL;
	sscanf(fen, "%s %c %s %s %d %d", pos, &mv, cas, enps, &halfm, &fullm);
	while ((c = pos[j++])) {
		if (c == '/') {
			row--;
			col = 0;
		} else if (c >= '1' && c <= '8') {
			col += c - '0';
		} else for (i = 1; i < 8; i++) {
			if (pieceChar[i] == c) {
				if (i == KING) kingpos[0] = row*8 + col;
				hashb ^= hashxor[0 | (row << 1) | (i << 4) | (col << 7)];
				setBit(row*8 + col, pieceb+i);
				setBit(row*8 + (col++), colorb);
				break;
			} else if (pieceChar[i] == c - 32) {
				if (i == KING) kingpos[1] = row*8 + col;
				hashb ^= hashxor[1 | (row << 1) | (i << 4) | (col << 7)];
				setBit(row*8 + col, pieceb+i);
				setBit(row*8 + (col++), colorb+1);
				break;
			}
		}
	}
	onmove = mv == 'b' ? 1 : 0;
	flags = j = 0;
	while ((c = cas[j++])) {
		if (c == 'K') flags |= BIT[6];
		if (c == 'k') flags |= BIT[7];
		if (c == 'Q') flags |= BIT[8];
		if (c == 'q') flags |= BIT[9];
	}
	if (enps[0] != '-') {
		flags |= 8*(enps[1] - '1') + enps[0] - 'a'; 
	}
	board = colorb[0] | colorb[1];
}

static u32 r_x = 30903, r_y = 30903, r_z = 30903, r_w = 30903, r_carry = 0;
u32 _rand_32() {
	u32 t;
	r_x = r_x * 69069 + 1;
	r_y ^= r_y << 13;
	r_y ^= r_y >> 17;
	r_y ^= r_y << 5;
	t = (r_w << 1) + r_z + r_carry;
	r_carry = ((r_z >> 2) + (r_w >> 3) + (r_carry >> 2)) >> 30;
	r_z = r_w;
	r_w = t;
	return r_x + r_y + r_w;
}

u64 _rand_64() { u64 c = _rand_32(); return _rand_32() | (c << 32); }

u64 getTime()
{
#ifdef _WIN32
	_ftime(&tv);
	return(tv.time * 1000LL + tv.millitm);
#else
	gettimeofday (&tv, &tz);
	return(tv.tv_sec * 1000LL + (tv.tv_usec / 1000));
#endif
}

char getLsb(u64 bm) {
	u32 n = (u32) (bm & 0xFFFFFFFF);
	if (n) {
		if (n & 0xFFFF) return LSB[n & 0xFFFF];
		else return 16 | LSB[(n >> 16) & 0xFFFF];
	} else {
		n = (u32)(bm >> 32);
		if (n & 0xFFFF) return 32 | LSB[n & 0xFFFF];
		else return 48 | LSB[(n >> 16) & 0xFFFF];
	}
}

char _slow_lsb(u64 bm) {
	int k = -1;
	while (bm) { k++; if (bm & 1) break; bm >>= 1; }
	return (char)k;
}

int pullLsb(u64* bit) {
	int f = getLsb(*bit);
	*bit ^= BIT[f];
	return f;
}

int _bitcount(u64 bit) {
	int count=0;
	while (bit) { bit &= (bit-1); count++; }
	return count;
}

int bitcount (u64 n)
{    
     return BITC[n         & 0xFFFF]
         +  BITC[(n >> 16) & 0xFFFF]
         +  BITC[(n >> 32) & 0xFFFF]
         +  BITC[(n >> 48) & 0xFFFF];
}

int identPiece(int f) {
	if (TEST(f, pieceb[PAWN])) return PAWN;
	if (TEST(f, pieceb[KNIGHT])) return KNIGHT;
	if (TEST(f, pieceb[BISHOP])) return BISHOP;
	if (TEST(f, pieceb[ROOK])) return ROOK;
	if (TEST(f, pieceb[QUEEN])) return QUEEN;
	if (TEST(f, pieceb[KING])) return KING;
	return ENP;
}

int identColor(int f) {
	if (TEST(f, colorb[1])) return 1;
	return 0;
}

u64 bmask45[64];
u64 bmask135[64];

int key000(u64 b, int f) {
	return (int) ((b >> (f & 56)) & 0x7E);
}

#if defined(_WIN64) || defined(_LP64)
int key090(u64 b, int f) {
	u64 _b = (b >> (f&7)) & 0x0101010101010101LL;
	_b = _b * 0x0080402010080400LL;
	return (int)(_b >> 57);
}

int keyDiag(u64 _b) {
	_b = _b * 0x0202020202020202LL;
	return (int)(_b >> 57);
}
#else
int key090(u64 b, int f) {
	int h;
	b = b >> (f&7);
	h = (int)((b & 0x1010101) | ((b >> 31) & 0x2020202));
	h = (h & 0x303) | ((h >> 14) & 0xC0C);
	return (h & 0xE) | ((h >> 4) & 0x70);
}

int keyDiag(u64 _b) {
   int h = (int)(_b | _b >> 32);
   h |= h >> 16;
   h |= h >>  8;
   return h & 0x7E;
}
#endif

int key045(u64 b, int f) {
   return keyDiag(b & bmask45[f]);
}

int key135(u64 b, int f) {
   return keyDiag(b & bmask135[f]);
}

#define DUALATT(x, y, c) (battacked(x, c) || battacked(y, c))
#define RQU (pieceb[ROOK] | pieceb[QUEEN])
#define BQU (pieceb[BISHOP] | pieceb[QUEEN])

int battacked(int f, int c) {
	if (PCAP(f, c) & pieceb[PAWN]) return 1;
	if (NCAP(f, c) & pieceb[KNIGHT]) return 1;
	if (KCAP(f, c) & pieceb[KING]) return 1;
	if (RCAP1(f, c) & RQU) return 1; 
	if (RCAP2(f, c) & RQU) return 1; 
	if (BCAP3(f, c) & BQU) return 1;
	if (BCAP4(f, c) & BQU) return 1;
	return 0;
}

u64 reach(int f, int c) {
	return (NCAP(f, c) & pieceb[KNIGHT])
		| (RCAP1(f, c) & RQU)
		| (RCAP2(f, c) & RQU)
		| (BCAP3(f, c) & BQU)
		| (BCAP4(f, c) & BQU);
}

u64 attacked(int f, int c) {
	return (PCAP(f, c) & pieceb[PAWN]) | reach(f, c);
}

void _init_pawns(u64* moves, u64* caps, int c) {
	int i;
	for (i = 0; i < 64; i++) {
		int m = i + (c ? -8 : 8);
		if (m < 0 || m > 63) continue;
		setBit(m, moves + i);
		if ((i&7) > 0) {
			m = i + (c ? -9 : 7);
			if (m < 0 || m > 63) continue;
			setBit(m, caps + i);
			setBit(m, caps + i + 128*(2 - c));
		}
		if ((i&7) < 7) {
			m = i + (c ? -7 : 9);
			if (m < 0 || m > 63) continue;
			setBit(m, caps + i);
			setBit(m, caps + i + 128*(c + 1));
		}
	}
}

void _init_shorts(u64* moves, int* m) {
	int i, j, n;
	for (i = 0; i < 64; i++) {
		for (j = 0; j < 8; j++) {
			n = i + m[j];
			if (n < 64 && n >= 0 && ((n & 7)-(i & 7))*((n & 7)-(i & 7)) <= 4) {
				setBit(n, moves+i);
			}
		}
	}
}

u64 _occ_free_board(int bc, int del, u64 free) {
	u64 low, perm = free;
	int i;
	for (i = 0; i < bc; i++) {
		low = getLowestBit(free);
		free &= (~low);
		if (!TEST(i, del)) perm &= (~low);
	}
	return perm;
}

void _init_rays(u64* rays, u64 (*rayFunc) (int, u64, int), int (*key)(u64, int)) {
	int i, f, iperm, bc, index; 
	u64 board, mmask, occ, move, xray;
	for (f = 0; f < 64; f++) {
		mmask = (*rayFunc)(f, 0LL, 0) | BIT[f];
		iperm = 1 << (bc = bitcount(mmask));
		for (i = 0; i < iperm; i++) {
			board = _occ_free_board(bc, i, mmask);
			move = (*rayFunc)(f, board, 1);
			occ = (*rayFunc)(f, board, 2);
			xray = (*rayFunc)(f, board, 3);
			index = (*key)(board, f);
			rays[(f << 7) + index] = occ | move;
			rays[(f << 7) + index + 0x8000] = xray;
		}
	}
}

#define RAYMACRO {if (TEST(i, board)) { if (b) { setBit(i, &xray); break; } else { setBit(i, &occ); b = 1; } } if (!b) setBit(i, &free);}
u64 _rook0(int f, u64 board, int t) {
	u64 free = 0LL, occ = 0LL, xray = 0LL;
	int i, b;
	for (b = 0, i = f+1; i < 64 && i%8 != 0; i++) RAYMACRO
	for (b = 0, i = f-1; i >= 0 && i%8 != 7; i--) RAYMACRO
	return (t < 2) ? free : (t == 2 ? occ : xray);
}

u64 _rook90(int f, u64 board, int t) {
	u64 free = 0LL, occ = 0LL, xray = 0LL;
	int i, b;
	for (b = 0, i = f-8; i >= 0; i-=8) RAYMACRO
	for (b = 0, i = f+8; i < 64; i+=8) RAYMACRO
	return (t < 2) ? free : (t == 2 ? occ : xray);
}

u64 _bishop45(int f, u64 board, int t) {
	u64 free = 0LL, occ = 0LL, xray = 0LL;
	int i, b;
	for (b = 0, i = f+9; i < 64 && (i%8 != 0); i+=9) RAYMACRO
		for (b = 0, i = f-9; i >= 0 && (i%8 != 7); i-=9) RAYMACRO
			return (t < 2) ? free : (t == 2 ? occ : xray);
}

u64 _bishop135(int f, u64 board, int t) {
	u64 free = 0LL, occ = 0LL, xray = 0LL;
	int i, b;
	for (b = 0, i = f-7; i >= 0 && (i%8 != 0); i-=7) RAYMACRO
		for (b = 0, i = f+7; i < 64 && (i%8 != 7); i+=7) RAYMACRO
			return (t < 2) ? free : (t == 2 ? occ : xray);
}

//void display64(u64 bb) {
//	int i, j;
//	for (i = 0; i < 8; i++) {
//		for (j = 0; j < 8; j++) {
//			printf(" %d", TEST(j + (7-i)*8, bb) ? 1 : 0);
//		}
//		printf("\n");
//	}
//	printf("\n");
//}

void displayBoard() {
	int i, j;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			int f = j + (7-i)*8;
			printf(" %c", pieceChar[identPiece(f)] + identColor(f)*32);
		}
		printf("\n");
	}
	printf("\n");
}

void displaym(Move m) {
	printf("%c%c%c%c%c%c", pieceChar[PIECE(m)], 'a' + FROM(m) % 8, '1' + FROM(m) / 8,
		CAP(m) == 0 ? '-' : 'x', 'a' + TO(m) % 8, '1' + TO(m) / 8);
	if (PROM(m)) printf("%c", pieceChar[PROM(m)]+32);
}

Move movestack[128];
void displaypv(int ply) {
	int i;
	for (i = 0; i < ply; i++) {
		displaym(movestack[i]); printf(" ");
	}
	printf("\n");
}

void doMove(Move m, int c) {
	int f = FROM(m);
	int t = TO(m);
	int p = PIECE(m);
	int a = CAP(m);

	xorBit(f, colorb+c);
	xorBit(f, pieceb+p);

	xorBit(t, colorb+c);
	xorBit(t, pieceb+p);
	flags &= 960;
	if (a) {
		if (a == ENP) { // Enpassant Capture
			t = (t&7) | (f&56);
			a = PAWN;
		} else if (a == ROOK && CASTLE) { //Revoke castling rights.
			flags &= crevoke[t];
		}
		xorBit(t, pieceb+a);
		xorBit(t, colorb+(c^1));
	}
	if (p == PAWN) {
		if (((f^t)&8) == 0) flags |= f^24; //Enpassant
		else if ((t&56) == 0 || (t&56) == 56) {
			xorBit(t, pieceb+PAWN);
			xorBit(t, pieceb+PROM(m));
		}
	} else if (p == KING) {
		if (kingpos[c] == f) kingpos[c] = t; else kingpos[c] = f;
		flags &= ~(320 << c); // Lose castling rights
		if (((f^t)&3) == 2) { // Castle
			if (t == 6) { f = 7; t = 5; }
			else if (t == 2) { f = 0; t = 3; }
			else if (t == 62) { f = 63; t = 61; }
			else { f = 56; t = 59; }
			xorBit(f, colorb+c);
			xorBit(f, pieceb+ROOK);
			xorBit(t, colorb+c);
			xorBit(t, pieceb+ROOK);
			hashb ^= hashxor[f | c << 12 | ROOK << 13];
			hashb ^= hashxor[t | c << 12 | ROOK << 13];
		}
	} else if (p == ROOK && CASTLE) {
		flags &= crevoke[f];
	}
	board = colorb[0] | colorb[1];
	hashb ^= hashxor[f | ((m >> 6) & 0x3C0)];
	hashb ^= hashxor[m >> 6];
}

void registerCaps(Move m, u64 bc, int* mlist, int* mn) {
	while (bc) {
		int t = pullLsb(&bc);
		mlist[(*mn)++] = m | _TO(t) | _CAP(identPiece(t));
	}
}

void registerMoves(Move m, u64 bc, u64 bm, int* mlist, int* mn) {
	while (bc) {
		int t = pullLsb(&bc);
		mlist[(*mn)++] = m | _TO(t) | _CAP(identPiece(t));
	}
	while (bm) {
		mlist[(*mn)++] = m | _TO(pullLsb(&bm));
	}
}

void registerProms(int f, int c, u64 bc, u64 bm, int* mlist, int* mn) {
	while (bc) {
		int t = pullLsb(&bc);
		Move m = f | _ONMV(c) | _PIECE(PAWN) | _TO(t) | _CAP(identPiece(t));
		mlist[(*mn)++] = m | _PROM(QUEEN);
		mlist[(*mn)++] = m | _PROM(KNIGHT);
		mlist[(*mn)++] = m | _PROM(ROOK);
		mlist[(*mn)++] = m | _PROM(BISHOP);
	}
	while (bm) {
		Move m = f | _ONMV(c) | _PIECE(PAWN) | _TO(pullLsb(&bm));
		mlist[(*mn)++] = m | _PROM(QUEEN);
		mlist[(*mn)++] = m | _PROM(KNIGHT);
		mlist[(*mn)++] = m | _PROM(ROOK);
		mlist[(*mn)++] = m | _PROM(BISHOP);
	}
}

void registerKing(Move m, u64 bc, u64 bm, int* mlist, int* mn, int c) {
	while (bc) {
		int t = pullLsb(&bc);
		if (battacked(t, c)) continue;
		mlist[(*mn)++] = m | _TO(t) | _CAP(identPiece(t));
	}
	while (bm) {
		int t = pullLsb(&bm);
		if (battacked(t, c)) continue;
		mlist[(*mn)++] = m | _TO(t);
	}
}

char getDir(int f, int t) {
	if (!((f ^ t) & 56)) return 8;
	if (!((f ^ t) & 7)) return 16;
	return ((f - t) % 7) ? 32 : 64;
}

int generateCheckEsc(u64 ch, u64 apin, int c, int k, int *ml, int *mn) {
	u64 cc, fl;
	int d, bf = _bitcount(ch);
	board ^= BIT[k];
	registerKing(PREMOVE(k, KING), KCAP(k, c), KMOVE(k), ml, mn, c);
	board ^= BIT[k];
	if (bf > 1) return bf; //Multicheck
	bf = getLsb(ch);

	cc = attacked(bf, c^1) & apin;  //Can we capture the checker?
	while (cc) {
		int cf = pullLsb(&cc);
		int p = identPiece(cf);
		if (p == PAWN && RANK(cf, c ? 0x08 : 0x30)) {
			registerProms(cf, c, ch, 0LL, ml, mn);
		} else {
			registerMoves(PREMOVE(cf, p), ch, 0LL, ml, mn);
		}
	}
	if (ENPASS && (ch & pieceb[PAWN])) { //Enpassant capture of attacking Pawn
		cc = PCAP(ENPASS, c^1) & pieceb[PAWN] & apin;
		while (cc) {
			int cf = pullLsb(&cc);
			registerMoves(PREMOVE(cf, PAWN), BIT[ENPASS], 0LL, ml, mn);
		}
	}
	if (ch & (nmoves[k] | kmoves[k])) return 1; // We can't move anything between!

	d = getDir(bf, k);
	if (d & 8) fl = RMOVE1(bf) & RMOVE1(k);
	else if (d & 16) fl = RMOVE2(bf) & RMOVE2(k);
	else if (d & 32) fl = BMOVE3(bf) & BMOVE3(k);
	else fl = BMOVE4(bf) & BMOVE4(k);

	while (fl) {
		int f = pullLsb(&fl);
		cc = reach(f, c^1) & apin;
		while (cc) {
			int cf = pullLsb(&cc);
			int p = identPiece(cf);
			registerMoves(PREMOVE(cf, p), 0LL, BIT[f], ml, mn);
		}
		bf = c ? f+8 : f-8;
		if (bf < 0 || bf > 63) continue;
		if (BIT[bf] & pieceb[PAWN] & colorb[c] & apin) {
			if (RANK(bf, c ? 0x08 : 0x30))
				registerProms(bf, c, 0LL, BIT[f], ml, mn);
			else
				registerMoves(PREMOVE(bf, PAWN), 0LL, BIT[f], ml, mn);
		}
		if (RANK(f, c ? 0x20 : 0x18) && (board & BIT[bf]) == 0 && (BIT[c ? f+16 : f-16] & pieceb[PAWN] & colorb[c] & apin))
			registerMoves(PREMOVE(c ? f+16 : f-16, PAWN), 0LL, BIT[f], ml, mn);
	}
	return 1;
}

u64 pinnedPieces(int f, int oc) {
	u64 pin = 0LL;
	u64 b = ((RXRAY1(f) | RXRAY2(f)) & colorb[oc]) & RQU;
	while (b) {
		int t = pullLsb(&b);
		pin |= RCAP(t, oc) & ROCC(f);
	}
	b = ((BXRAY3(f) | BXRAY4(f)) & colorb[oc]) & BQU;
	while (b) {
		int t = pullLsb(&b);
		pin |= BCAP(t, oc) & BOCC(f);
	}
	return pin;
}

int generateMoves(int c, int ply) {
	int t, f = kingpos[c];
	int *mn = movenum + ply;
	int *ml = movelist + (ply << 8);
	u64 m, b, a, cb = colorb[c], ch = attacked(f, c);
	u64 pin = pinnedPieces(f, c^1);
	*mn = 0;

	if (ch) return generateCheckEsc(ch, ~pin, c, f, ml, mn);
	registerKing(PREMOVE(f, KING), KCAP(f, c), KMOVE(f), ml, mn, c);

	cb = colorb[c] & (~pin);
	b = pieceb[PAWN] & cb;
	while (b) {
		f = pullLsb(&b);
		m = PMOVE(f, c);
		a = PCAP(f, c);
		if (m && RANK(f, c ? 0x30 : 0x08)) m |= PMOVE(c ? f-8 : f+8, c);
		if (RANK(f, c ? 0x08 : 0x30)) {
			registerProms(f, c, a, m, ml, mn);
		} else {
			if (ENPASS && (BIT[ENPASS] & pcaps[(f) | ((c)<<6)])) {
				u64 hh;
				int clbd = ENPASS^8;
				board ^= BIT[clbd];
				hh = ROCC1(f);
				if (!(hh & BIT[kingpos[c]]) || !(hh & colorb[c^1] & RQU)) {
					a = a | BIT[ENPASS];
				}
				board ^= BIT[clbd];
			}
			registerMoves(PREMOVE(f, PAWN), a, m, ml, mn);
		}
	}

	b = pin & pieceb[PAWN]; 
	while (b) {
		f = pullLsb(&b);
		t = getDir(f, kingpos[c]);
		if (t & 8) continue;
		m = a = 0LL;
		if (t & 16) {
			m = PMOVE(f, c);         
			if (m && RANK(f, c ? 0x30 : 0x08)) m |= PMOVE(c ? f-8 : f+8, c);
		} else if (t & 32) {
			a = PCA3(f, c);
		} else {
			a = PCA4(f, c);
		}
		if (RANK(f, c ? 0x08 : 0x30)) {
			registerProms(f, c, a, m, ml, mn);
		} else {
			registerMoves(PREMOVE(f, PAWN), a, m, ml, mn);
		}
	}

	b = pieceb[KNIGHT] & cb;
	while (b) {
		f = pullLsb(&b);
		registerMoves(PREMOVE(f, KNIGHT), NCAP(f, c), NMOVE(f), ml, mn);
	}

	b = pieceb[ROOK] & cb;
	while (b) {
		f = pullLsb(&b);
		registerMoves(PREMOVE(f, ROOK), RCAP(f, c), RMOVE(f), ml, mn);
		if (CASTLE && !ch) {
			if (c) {
				if ((flags & 128) && (f == 63) && (RMOVE1(63) & BIT[61]))
					if (!DUALATT(61, 62, c)) registerMoves(PREMOVE(60, KING), 0LL, BIT[62], ml, mn);
				if ((flags & 512) && (f == 56) && (RMOVE1(56) & BIT[59]))
					if (!DUALATT(59, 58, c)) registerMoves(PREMOVE(60, KING), 0LL, BIT[58], ml, mn);
			} else {
				if ((flags & 64) && (f == 7) && (RMOVE1(7) & BIT[5]))
					if (!DUALATT(5, 6, c)) registerMoves(PREMOVE(4, KING), 0LL, BIT[6], ml, mn);
				if ((flags & 256) && (f == 0) && (RMOVE1(0) & BIT[3]))
					if (!DUALATT(3, 2, c)) registerMoves(PREMOVE(4, KING), 0LL, BIT[2], ml, mn);
			}
		}
	}

	b = pieceb[BISHOP] & cb;
	while (b) {
		f = pullLsb(&b);
		registerMoves(PREMOVE(f, BISHOP), BCAP(f, c), BMOVE(f), ml, mn);
	}

	b = pieceb[QUEEN] & cb;
	while (b) {
		f = pullLsb(&b);
		registerMoves(PREMOVE(f, QUEEN), RCAP(f, c) | BCAP(f,c), RMOVE(f) | BMOVE(f), ml, mn);
	}

	b = pin & (pieceb[ROOK] | pieceb[BISHOP] | pieceb[QUEEN]); 
	while (b) {
		int p;
		f = pullLsb(&b);
		p = identPiece(f);
		t = p | getDir(f, kingpos[c]);
		if ((t & 10) == 10) registerMoves(PREMOVE(f, p), RCAP1(f, c), RMOVE1(f), ml, mn);
		if ((t & 18) == 18) registerMoves(PREMOVE(f, p), RCAP2(f, c), RMOVE2(f), ml, mn);
		if ((t & 33) == 33) registerMoves(PREMOVE(f, p), BCAP3(f, c), BMOVE3(f), ml, mn);
		if ((t & 65) == 65) registerMoves(PREMOVE(f, p), BCAP4(f, c), BMOVE4(f), ml, mn);
	}
	return 0;
}

void countKing(u64 bm, int* mn, int c) {
	while (bm) {
		if (!battacked(pullLsb(&bm), c)) (*mn)++;
	}
}

int countMoves(int c, int ply) {
	int t, f = kingpos[c];
	int* mn = movenum + ply;
	int* ml = movelist + (ply << 8);
	u64 m, b, a, cb = colorb[c], ch = attacked(f, c);
	u64 pin = pinnedPieces(f, c^1);
	*mn = 0;

	if (ch) return generateCheckEsc(ch, ~pin, c, f, ml, mn);
	countKing(KCAP(f, c) | KMOVE(f), mn, c);

	cb = colorb[c] & (~pin);
	b = pieceb[PAWN] & cb;
	while (b) {
		f = pullLsb(&b);
		m = PMOVE(f, c);
		a = PCAP(f, c);
		if (m && RANK(f, c ? 0x30 : 0x08)) m |= PMOVE(c ? f-8 : f+8, c);
		if (RANK(f, c ? 0x08 : 0x30)) {
			*mn += _bitcount(a | m) << 2;
		} else {
			if (ENPASS && (BIT[ENPASS] & pcaps[(f) | ((c)<<6)])) {
				u64 hh;
				int clbd = ENPASS^8;
				board ^= BIT[clbd];
				hh = ROCC1(f);
				if (!(hh & BIT[kingpos[c]]) || !(hh & colorb[c^1] & RQU)) {
					a = a | BIT[ENPASS];
				}
				board ^= BIT[clbd];
			}
			*mn += _bitcount(a | m);
		}
	}

	b = pin & pieceb[PAWN]; 
	while (b) {
		f = pullLsb(&b);
		t = getDir(f, kingpos[c]);
		if (t & 8) continue;
		m = a = 0LL;
		if (t & 16) {
			m = PMOVE(f, c);         
			if (m && RANK(f, c ? 0x30 : 0x08)) m |= PMOVE(c ? f-8 : f+8, c);
		} else if (t & 32) {
			a = PCA3(f, c);
		} else {
			a = PCA4(f, c);
		}
		if (RANK(f, c ? 0x08 : 0x30)) {
			*mn += _bitcount(a | m) << 2;
		} else {
			*mn += _bitcount(a | m);
		}
	}

	b = pieceb[KNIGHT] & cb;
	while (b) {
		f = pullLsb(&b);
		*mn += bitcount(NCAP(f, c) | NMOVE(f));
	}

	b = pieceb[BISHOP] & cb;
	while (b) {
		f = pullLsb(&b);
		*mn += bitcount(BCAP(f,c) | BMOVE(f));
	}

	b = pieceb[ROOK] & cb;
	while (b) {
		f = pullLsb(&b);
		*mn += bitcount(RCAP(f,c) | RMOVE(f));
		if (CASTLE && !ch) {
			if (c) {
				if ((flags & 128) && (f == 63) && (RMOVE1(63) & BIT[61]))
					if (!DUALATT(61, 62, c)) (*mn)++;
				if ((flags & 512) && (f == 56) && (RMOVE1(56) & BIT[59]))
					if (!DUALATT(59, 58, c)) (*mn)++;
			} else {
				if ((flags & 64) && (f == 7) && (RMOVE1(7) & BIT[5]))
					if (!DUALATT(5, 6, c)) (*mn)++;
				if ((flags & 256) && (f == 0) && (RMOVE1(0) & BIT[3]))
					if (!DUALATT(3, 2, c)) (*mn)++;
			}
		}
	}

	b = pieceb[QUEEN] & cb;
	while (b) {
		f = pullLsb(&b);
		*mn += bitcount(RCAP(f, c) | BCAP(f, c) | RMOVE(f) | BMOVE(f));
	}

	b = pin & (pieceb[ROOK] | pieceb[BISHOP] | pieceb[QUEEN]); 
	while (b) {
		int p;
		f = pullLsb(&b);
		p = identPiece(f);
		t = p | getDir(f, kingpos[c]);
		if ((t & 10) == 10) *mn += _bitcount(RCAP1(f, c) | RMOVE1(f));
		if ((t & 18) == 18) *mn += _bitcount(RCAP2(f, c) | RMOVE2(f));
		if ((t & 33) == 33) *mn += _bitcount(BCAP3(f, c) | BMOVE3(f));
		if ((t & 65) == 65) *mn += _bitcount(BCAP4(f, c) | BMOVE4(f));
	}
	return 0;
}

#define HASHB(d) (hashb ^ hashxor[flags | (c) << 10 | (d) << 11])
#define HSIZE 0x400000
#define HMASK 0x3FFFFF
#define HINV 0xFFFFFFFFFFC00000LL
u64 hashDB[HSIZE];
u64 num[128];

void perft(int c, int d, int ply) {
	int i, poff;
	int flagstor = flags;

	u64 hb = HASHB(d);
	u64 he = hashDB[hb & HMASK];
	u64 n0, n1 = he & HMASK;

	if (!((he^hb) & HINV)) {
		num[ply+d] += n1;
		return;
	}

	if (d<= 1) {
		countMoves(c, ply);
		if (movenum[ply] > n1) hashDB[hb & HMASK] = (hb & HINV) | movenum[ply]; 
		num[ply+1]+= movenum[ply];
		return;
	}
	n0 = num[ply+d];

	generateMoves(c, ply);
	poff = ply << 8;
	for (i = 0; i < movenum[ply]; i++) {
		Move m = movelist[poff + i];			
//		movestack[ply] = m;
		doMove(m, c);

		num[ply+1]++;
		if (d > 1) perft(c^1, d-1, ply+1);

		doMove(m, c);
		flags = flagstor;
	}
	n0 = num[ply+d] - n0;
	if (n0 < 0x100000 && n0 > n1) {
		hashDB[hb & HMASK] = (hb & HINV) | n0; 
	}
}

int main(int argc, char **argv)
{
	int i, divide = 0, sd = 6;
	u64 t1, n = 0;
	char *sfen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	if (argc > 1) sscanf(argv[1], "%d", &sd);
	if (argc > 2) sfen = argv[2];
	if (sd < 0) {
		sd = -sd;
		divide = 1;
	}
	for (i = 0; i < 0x10000; i++) LSB[i] = _slow_lsb(i);
	for (i = 0; i < 0x10000; i++) hashxor[i] = _rand_64();
	for (i = 0; i < 0x10000; i++) BITC[i] = _bitcount(i);
	for (i = 0; i < HSIZE; i++) hashDB[i] = 0LL;
	for (i = 0; i < 64; i++) BIT[i] = 1LL << i;
	for (i = 0; i < 64; i++) crevoke[i] = 0x3FF;
	for (i = 0; i < 128; i++) pmoves[i] = 0LL;
	for (i = 0; i < 384; i++) pcaps[i] = 0LL;
	crevoke[7] ^= BIT[6];
	crevoke[63] ^= BIT[7];
	crevoke[0] ^= BIT[8];
	crevoke[56] ^= BIT[9];

	for (i = 0; i < 64; i++) bmask45[i] = _bishop45(i, 0LL, 0) | BIT[i];
	for (i = 0; i < 64; i++) bmask135[i] = _bishop135(i, 0LL, 0) | BIT[i];

	_init_rays(rays, _rook0, key000);
	_init_rays(rays + 0x2000, _rook90, key090);
	_init_rays(rays + 0x4000, _bishop45, key045);
	_init_rays(rays + 0x6000, _bishop135, key135);
	_init_shorts(nmoves, _knight);
	_init_shorts(kmoves, _king);
	_init_pawns(pmoves, pcaps, 0);
	_init_pawns(pmoves + 64, pcaps + 64, 1);
	_parse_fen(sfen);
        displayBoard();

	t1 = getTime();

	if (divide) {
		int flagstor = flags;
		generateMoves(onmove, 0);
		for (i = 0; i < movenum[0]; i++) {
			Move m = movelist[i];
			doMove(m, onmove);

			num[1]++;
			perft(onmove^1, sd-1, 1);

			displaym(m); printf(": %llu\n", num[sd]);
			doMove(m, onmove);
			flags = flagstor;
			n += num[sd];
			num[sd] = 0;
		}
	} else {
		for (i = 1; i <= sd; i++) {
			num[i] = 0;
			perft(onmove, i, 0);
			n += num[i];
			printf("%2d %5d %6llu %11llu\n", i, 0, (getTime() - t1)/10, num[i]);
		}
	}
	t1 = getTime() - t1 + 1;
	printf("\nNodes: %llu cs: %llu knps: %llu\n", n, t1/10, n/t1);
	return 0;
}

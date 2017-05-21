//#include <utility/poly1305.h>
//
//__USING_SYS;
//
//// 2^(130) - 5
//const unsigned char Poly1305::p1305_data[17] = { 251,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,3 };
//const Poly1305::digit Poly1305::default_barrett_u[] = {0, 1342177280U, 0, 0, 0, 1073741824U};
//
//Poly1305::Poly1305(char __k[16], char __r[16], Cipher *c)
//{
//	this->k(__k); this->r(__r); this->cipher(c);
//	unsigned int i;
//	for(i=0;i<17;i++)
//		p1305[i] = p1305_data[i];
//	for(;i<sz_word;i++)
//		p1305[i] = 0;
//	_mod = (digit *)p1305;
//	_barrett_u = default_barrett_u;
//}
//
//Poly1305::Poly1305(char __k[16], char __r[16])
//{
//	this->k(__k); this->r(__r);
//	unsigned int i;
//	for(i=0;i<17;i++)
//		p1305[i] = p1305_data[i];
//	for(;i<sz_word;i++)
//		p1305[i] = 0;
//	_mod = (digit *)p1305;
//	_barrett_u = default_barrett_u;
//}
//
//Poly1305::Poly1305(Cipher *c)
//{
//	this->cipher(c);
//	unsigned int i;
//	for(i=0;i<17;i++)
//		p1305[i] = p1305_data[i];
//	for(;i<sz_word;i++)
//		p1305[i] = 0;
//	_mod = (digit *)p1305;
//	_barrett_u = default_barrett_u;
//}
//
//Poly1305::Poly1305()
//{
//	_cipher = new AES();
//	unsigned int i;
//	for(i=0;i<17;i++)
//		p1305[i] = p1305_data[i];
//	for(;i<sz_word;i++)
//		p1305[i] = 0;
//	_mod = (digit *)p1305;
//	_barrett_u = default_barrett_u;
//}
//
//Poly1305::~Poly1305(){}
//
//void Poly1305::k(char k1[16])
//{
//	unsigned int i;
//	for(i=0; i<16; i++)
//		_k[i] = k1[i];
//	for(; i<sz_word; i++)
//		_k[i] = 0;
//}
//
//void Poly1305::r(char r1[16])
//{
//	unsigned int i;
//	for(i=0; i<16; i++)
//		_r[i] = r1[i];
//	for(; i<sz_word; i++)
//		_r[i] = 0;
//
//	_r[3] &= 15;
//	_r[7] &= 15;
//	_r[11] &= 15;
//	_r[15] &= 15;
//	_r[4] &= 252;
//	_r[8] &= 252;
//	_r[12] &= 252;
//}
//
//void Poly1305::kr(char kr[32])
//{
//	this->k(kr);
//	this->r(kr+16);
//}
//
//bool Poly1305::isequal(const char x[16], const char y[16])
//{
//	register unsigned int d; register unsigned int x0;
//	register unsigned int x1; register unsigned int x2;
//	register unsigned int x3; register unsigned int x4;
//	register unsigned int x5; register unsigned int x6;
//	register unsigned int x7; register unsigned int x8;
//	register unsigned int x9; register unsigned int x10;
//	register unsigned int x11; register unsigned int x12;
//	register unsigned int x13; register unsigned int x14;
//	register unsigned int x15; register unsigned int y0;
//	register unsigned int y1; register unsigned int y2;
//	register unsigned int y3; register unsigned int y4;
//	register unsigned int y5; register unsigned int y6;
//	register unsigned int y7; register unsigned int y8;
//	register unsigned int y9; register unsigned int y10;
//	register unsigned int y11; register unsigned int y12;
//	register unsigned int y13; register unsigned int y14;
//	register unsigned int y15;
//
//	x0 = *(unsigned char *) (x + 0);
//	y0 = *(unsigned char *) (y + 0);
//	x1 = *(unsigned char *) (x + 1);
//	y1 = *(unsigned char *) (y + 1);
//	x2 = *(unsigned char *) (x + 2);
//	y2 = *(unsigned char *) (y + 2);
//	d = y0 ^ x0;
//	x3 = *(unsigned char *) (x + 3);
//	y1 ^= x1;
//	y3 = *(unsigned char *) (y + 3);
//	d |= y1;
//	x4 = *(unsigned char *) (x + 4);
//	y2 ^= x2;
//	y4 = *(unsigned char *) (y + 4);
//	d |= y2;
//	x5 = *(unsigned char *) (x + 5);
//	y3 ^= x3;
//	y5 = *(unsigned char *) (y + 5);
//	d |= y3;
//	x6 = *(unsigned char *) (x + 6);
//	y4 ^= x4;
//	y6 = *(unsigned char *) (y + 6);
//	d |= y4;
//	x7 = *(unsigned char *) (x + 7);
//	y5 ^= x5;
//	y7 = *(unsigned char *) (y + 7);
//	d |= y5;
//	x8 = *(unsigned char *) (x + 8);
//	y6 ^= x6;
//	y8 = *(unsigned char *) (y + 8);
//	d |= y6;
//	x9 = *(unsigned char *) (x + 9);
//	y7 ^= x7;
//	y9 = *(unsigned char *) (y + 9);
//	d |= y7;
//	x10 = *(unsigned char *) (x + 10);
//	y8 ^= x8;
//	y10 = *(unsigned char *) (y + 10);
//	d |= y8;
//	x11 = *(unsigned char *) (x + 11);
//	y9 ^= x9;
//	y11 = *(unsigned char *) (y + 11);
//	d |= y9;
//	x12 = *(unsigned char *) (x + 12);
//	y10 ^= x10;
//	y12 = *(unsigned char *) (y + 12);
//	d |= y10;
//	x13 = *(unsigned char *) (x + 13);
//	y11 ^= x11;
//	y13 = *(unsigned char *) (y + 13);
//	d |= y11;
//	x14 = *(unsigned char *) (x + 14);
//	y12 ^= x12;
//	y14 = *(unsigned char *) (y + 14);
//	d |= y12;
//	x15 = *(unsigned char *) (x + 15);
//	y13 ^= x13;
//	y15 = *(unsigned char *) (y + 15);
//	d |= y13;
//	y14 ^= x14;
//	d |= y14;
//	y15 ^= x15;
//	d |= y15;
//	d -= 1;
//	d >>= 8;
//
//	return (d != 0);
//}
//
//bool Poly1305::cipher(Cipher *c)
//{
//	bool ok = true;//(c->mode(Cipher::CBC));
//	if(ok) _cipher = c;
//	return ok;
//}
//
//// #define POLY_PRINT(x) kout << "["; for(int i=0;i<16;i++) kout << (unsigned int)x[i] << " "; kout << "]" << endl;
//bool Poly1305::verify(const char a[16],
//  const char n[16], const char m[],unsigned int l)
//{
//  char aeskn[16];
//  char valid[16];
//  aes(aeskn, n);
//  poly1305_bignum(valid,aeskn,m,l);
////   kout << "===========" << endl;
////   POLY_PRINT(a); POLY_PRINT(valid); POLY_PRINT(n); POLY_PRINT(m); POLY_PRINT(aeskn); POLY_PRINT(_k); POLY_PRINT(_r);
////   kout << "===========" << endl;
//  return isequal(a,valid);
//}
//
//void Poly1305::authenticate(char out[16],
//  const char n[16], const char m[],unsigned int l)
//{
//  char aeskn[16];
//  aes(aeskn, n);
//  poly1305_bignum(out,aeskn,m,l);
////   kout << "===========" << endl;
////   POLY_PRINT(out); POLY_PRINT(n); POLY_PRINT(m); POLY_PRINT(aeskn); POLY_PRINT(_k); POLY_PRINT(_r);
////   kout << "===========" << endl;
//}
//
//void Poly1305::poly1305_bignum(char *out, const char *s, const char *m, unsigned int l)
//{
///* r = Poly's key
//   s = AES result
//   m = message
//   l = message length */
//	unsigned int j;
//	unsigned int end=16;
//	unsigned char c[sz_word], h[(sz_word+1)*2];
//	/*
//	Chronometer t, mu, mo, bo, sl;
//	int cm, cmo, cb, csl;
//	unsigned long long tm, tmo, tb, tsl;
//	cm = cmo = cb = csl = 0;
//	tm = tmo = tb = tsl = 0;
//	t.start();
//	*/
//
//	for(unsigned int i=0;i<sizeof(h);i++)
//		h[i] = 0;
//
//	while(l>0)
//	{
//		if(l<end) end = l;
//
//  	//	csl+=1; sl.reset(); sl.start();
//		unsigned int i;
//		for(i=0; i<end; i++)
//			c[i] = m[i];
//		c[end] = 1;
//		for(;i<sz_word;i++)
//			c[i] = 0;
//  	//	sl.stop(); tsl+=sl.read();
//
//  	//	cmo+=1; mo.reset(); mo.start();
//		simple_add((digit *)h, (digit *)h, (digit *)c, word);
////			simple_sub((digit *)h, (digit *)h, _mod, word);
////		if(cmp((digit *)h, _mod, word) >= 0)
////			simple_sub((digit *)h, (digit *)h, _mod, word);
//  	//	mo.stop(); tmo+=mo.read();
//
//		m += end;
//		l -= end;
//
//  	//	cm++; mu.reset(); mu.start();
//		simple_mult((digit *)h, (digit *)h, (digit *)_r, word);
//  	//	mu.stop(); tm += mu.read();
//
//  	//	cb++; bo.reset(); bo.start();
//		barrett_reduction((digit *)h, (digit *)h, word);
//  	//	bo.stop(); tb += bo.read();
//	}
//
////	csl++; sl.reset(); sl.start();
//	unsigned int i;
//	for(i=0; i<16; i++)
//		c[i] = s[i];
//	for(;i<sz_word;i++)
//		c[i] = 0;
////	sl.stop(); tsl+=sl.read();
//
////	cmo++; mo.reset(); mo.start();
//	simple_add((digit *)h, (digit *)h, (digit *)c, word);
////	mo.stop(); tmo+=mo.read();
//
////	csl++; sl.reset(); sl.start();
//	for(j = 0;j < 16;j++)
//		out[j] = h[j];
////	sl.stop(); tsl+=sl.read();
//  //	t.stop();
//	/*
//	kout << "===Poly1305 operations times===\n";
//	kout << "Mult: " << cm << " ops in " << tm << "us"<<endl;
//	kout << "Add: " << cmo << " ops in " << tmo << "us"<<endl;
//	kout << "Div+Mod: "<< cb << " ops in "  << tb << "us"<<endl;
//	kout << "Set Data: "<< csl << " ops in "  << tsl << "us"<<endl;
//	kout << "Total ops: "<< cb+cm+cmo << " ops in "  << tb+tm+tmo << "us"<<endl;
//	kout << "Total time: "<< t.read() << "us"<<endl;
//	kout << "===============================\n";
//	*/
//}

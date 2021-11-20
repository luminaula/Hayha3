#include "gmath.hpp"

void transposematrix(float m[16], aiMatrix4x4 *p)
{
	m[0] = p->a1; m[4] = p->a2; m[8] = p->a3; m[12] = p->a4;
	m[1] = p->b1; m[5] = p->b2; m[9] = p->b3; m[13] = p->b4;
	m[2] = p->c1; m[6] = p->c2; m[10] = p->c3; m[14] = p->c4;
	m[3] = p->d1; m[7] = p->d2; m[11] = p->d3; m[15] = p->d4;
}

void extract3x3(aiMatrix3x3 *m3, aiMatrix4x4 *m4){
	m3->a1 = m4->a1; m3->a2 = m4->a2; m3->a3 = m4->a3;
	m3->b1 = m4->b1; m3->b2 = m4->b2; m3->b3 = m4->b3;
	m3->c1 = m4->c1; m3->c2 = m4->c2; m3->c3 = m4->c3;
}

void mixvector(aiVector3D *p, aiVector3D *a, aiVector3D *b, float t)
{
	p->x = a->x + t * (b->x - a->x);
	p->y = a->y + t * (b->y - a->y);
	p->z = a->z + t * (b->z - a->z);
}

float dotquaternions(aiQuaternion *a, aiQuaternion *b)
{
	return a->x*b->x + a->y*b->y + a->z*b->z + a->w*b->w;
}

void normalizequaternion(aiQuaternion *q)
{
	float d = sqrt(dotquaternions(q, q));
	if (d >= 0.00001) {
		d = 1 / d;
		q->x *= d;
		q->y *= d;
		q->z *= d;
		q->w *= d;
	} else {
		q->x = q->y = q->z = 0;
		q->w = 1;
	}
}

void mixquaternion(aiQuaternion *q, aiQuaternion *a, aiQuaternion *b, float t)
{
	aiQuaternion tmp;
	if (dotquaternions(a, b) < 0) {
		tmp.x = -a->x; tmp.y = -a->y; tmp.z = -a->z; tmp.w = -a->w;
		a = &tmp;
	}
	q->x = a->x + t * (b->x - a->x);
	q->y = a->y + t * (b->y - a->y);
	q->z = a->z + t * (b->z - a->z);
	q->w = a->w + t * (b->w - a->w);
	normalizequaternion(q);
}

void composematrix(aiMatrix4x4 *m, aiVector3D *t, aiQuaternion *q, aiVector3D *s)
{
	// quat to rotation matrix
	m->a1 = 1 - 2 * (q->y * q->y + q->z * q->z);
	m->a2 = 2 * (q->x * q->y - q->z * q->w);
	m->a3 = 2 * (q->x * q->z + q->y * q->w);
	m->b1 = 2 * (q->x * q->y + q->z * q->w);
	m->b2 = 1 - 2 * (q->x * q->x + q->z * q->z);
	m->b3 = 2 * (q->y * q->z - q->x * q->w);
	m->c1 = 2 * (q->x * q->z - q->y * q->w);
	m->c2 = 2 * (q->y * q->z + q->x * q->w);
	m->c3 = 1 - 2 * (q->x * q->x + q->y * q->y);

	// scale matrix
	m->a1 *= s->x; m->a2 *= s->x; m->a3 *= s->x;
	m->b1 *= s->y; m->b2 *= s->y; m->b3 *= s->y;
	m->c1 *= s->z; m->c2 *= s->z; m->c3 *= s->z;

	// set translation
	m->a4 = t->x; m->b4 = t->y; m->c4 = t->z;

	m->d1 = 0; m->d2 = 0; m->d3 = 0; m->d4 = 1;
}
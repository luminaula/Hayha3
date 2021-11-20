#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>



#include <GL/glew.h>
#include <GL/freeglut.h>


#ifndef GL_GENERATE_MIPMAP
#define GL_GENERATE_MIPMAP 0x8191
#endif

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

#ifndef GL_SAMPLE_ALPHA_TO_COVERAGE
#define GL_SAMPLE_ALPHA_TO_COVERAGE 0x809E
#endif

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "hcore.hpp"
#include "gmath.hpp"

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define CLAMP(x,a,b) MIN(MAX(x,a),b)

/*
 * Use Sean Barrett's excellent stb_image to load textures.
 */

#define STB_IMAGE_IMPLEMENTATION
   #include "stb_image.h"

struct aiScene *g_scene = NULL;
int lasttime = 0;
struct aiAnimation *curanim = NULL;
int animfps = 30, animlen = 0;
float animtick = 0;
int playing = 1;
int targetmesh = 0;

int showhelp = 0;
int doplane = 0;
int doalpha = 3;
int dowire = 0;
int dotexture = 1;
int dobackface = 1;
int dotwosided = 1;
int doperspective = 1;

int screenw = 4000, screenh = 4000;

unsigned int fbo, rbo_color, rbo_depth;

int mousex, mousey, mouseleft = 0, mousemiddle = 0, mouseright = 0;

int gridsize = 3;
float mindist = 1;
float maxdist = 10;

float light_position[4] = { -1, 2, 2, 0 };

char basedir[2000];

unsigned char checker_data[256*256];
unsigned int checker_texture = 0;

unsigned int imgcount = 0;
unsigned int maxImgCount = 0;
unsigned int generate = 0;
std::string savedir;// = "/media/xoxo/2f03a838-cd91-4e2b-bfbb-968314f77511/saved/idf/";
std::string prefix;

HCore::HCore *core;
HCore::HCore *auxCore;

unsigned char *zeros;

int vertexcount = 0, facecount = 0; // for statistics only

// opengl (and skinned vertex) buffers for the meshes
int meshcount = 0;
int curMesh = 0;
struct mesh {
	struct aiMesh *mesh;
	unsigned int texture;
	unsigned int maskTexture;
	unsigned int meshID;
	int vertexcount, elementcount;
	float *position;
	float *normal;
	float *texcoord;
	int *element;
} *meshlist = NULL;

bool drawMask = false;

#ifndef __cplusplus
typedef struct aiMatrix4x4 aiMatrix4x4;
typedef struct aiVector3D aiVector3D;
typedef struct aiQuaternion aiQuaternion;
#endif

using namespace cv;


Mat screenshot(){
	Mat img(screenh,screenw,CV_8UC4);
	glPixelStorei(GL_PACK_ALIGNMENT, (img.step & 3) ? 1 : 4);
	glPixelStorei(GL_PACK_ROW_LENGTH, img.step/img.elemSize());
	glReadPixels(0,0,img.cols,img.rows,GL_BGRA,GL_UNSIGNED_BYTE,img.data);
	return img;

}

int findXmin(const Mat &image){
    for(int x=0;x<image.cols;x++){
        for(int y=0;y<image.rows;y++){
            uint32_t alpha = image.at<Vec4b>(y,x)[3];
            if(alpha == 0xff)
                return x;
        }
    }
}

int findYmin(const Mat &image){
    for(int y=0;y<image.rows;y++){
        for(int x=0;x<image.cols;x++){
            uint32_t alpha = image.at<Vec4b>(y,x)[3];
            if(alpha == 0xff)
                return y;
        }
    }
}

int findXmax(const Mat &image){
    for(int i=image.cols-1;i>0;i--){
        for(int j=0;j<image.rows;j++){
            uint32_t alpha = image.at<Vec4b>(j,i)[3];
            if(alpha == 0xff)
                return i;
        }
    }
}

int findYmax(const Mat &image){
    for(int i=image.rows-1;i>0;i--){
        for(int j=0;j<image.cols;j++){
            uint32_t alpha = image.at<Vec4b>(i,j)[3];
            if(alpha == 0xff)
                return i;
        }
    }
}

Mat crop(const Mat &img){
	int xmi,ymi,xma,yma;

    auto xmin = auxCore->tPool->enqueue(findXmin,img);
    auto ymin = auxCore->tPool->enqueue(findYmin,img);
    auto xmax = auxCore->tPool->enqueue(findXmax,img);
    auto ymax = auxCore->tPool->enqueue(findYmax,img);
    
    xmin.wait();
    ymin.wait();
    xmax.wait();
    ymax.wait();

    xmi = xmin.get();
    xma = xmax.get();
    ymi = ymin.get();
    yma = ymax.get();

	Rect ROI(xmi,ymi,xma-xmi,yma-ymi);

	return img(ROI);
}


void initchecker(void)
{
	int x, y, i = 0;
	for (y = 0; y < 256; y++) {
		for (x = 0; x < 256; x++) {
			int k = ((x>>5) & 1) ^ ((y>>5) & 1);
			checker_data[i++] = k ? 255 : 192;
		}
	}
	glGenTextures(1, &checker_texture);
	glBindTexture(GL_TEXTURE_2D, checker_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, 1, 256, 256, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, checker_data);
}

void lowerstring(char *s)
{
	while (*s) { *s = tolower(*s); s++; }
}

unsigned int loadtexture(char *filename)
{
	unsigned int texture;
	unsigned char *image;
	int w, h, n, intfmt = 0, fmt = 0;

	image = stbi_load(filename, &w, &h, &n, 0);
	if (!image) {
		lowerstring(filename);
		image = stbi_load(filename, &w, &h, &n, 0);
		if (!image) {
			fprintf(stderr, "cannot load texture '%s'\n", filename);
			return 0;
		}
	}

	if (n == 1) { intfmt = fmt = GL_LUMINANCE; }
	if (n == 2) { intfmt = fmt = GL_LUMINANCE_ALPHA; }
	if (n == 3) { intfmt = fmt = GL_RGB; }
	if (n == 4) { intfmt = fmt = GL_RGBA; }

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, intfmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, image);
	//glGenerateMipmap(GL_TEXTURE_2D);

	free(image);

	return texture;
}

unsigned int loadmaterial(struct aiMaterial *material)
{
	char filename[2000];
	struct aiString str;
	if (!aiGetMaterialString(material, AI_MATKEY_TEXTURE_DIFFUSE(0), &str)) {
		char *s = strrchr(str.data, '/');
		if (!s) s = strrchr(str.data, '\\');
		if (!s) s = str.data; else s++;
		strcpy(filename, basedir);
		strcat(filename, s);
		return loadtexture(filename);
	}
	return 0;
}

/*
 * Some extra matrix, vector and quaternion functions.
 *
 * Many of these already exist in some variant in assimp,
 * but not exported to the C interface.
 */

// convert 4x4 to column major format for opengl


/*
 * Init, animate and draw aiScene.
 *
 * We build our own list of the meshes to keep the vertex data
 * in a format suitable for use with OpenGL. We also store the
 * resulting meshes during skeletal animation here.
 */



// find a node by name in the hierarchy (for anims and bones)
struct aiNode *findnode(struct aiNode *node, char *name)
{
	int i;
	if (!strcmp(name, node->mName.data))
		return node;
	for (i = 0; i < node->mNumChildren; i++) {
		struct aiNode *found = findnode(node->mChildren[i], name);
		if (found)
			return found;
	}
	return NULL;
}

// calculate absolute transform for node to do mesh skinning
void transformnode(aiMatrix4x4 *result, struct aiNode *node)
{
	if (node->mParent) {
		transformnode(result, node->mParent);
		aiMultiplyMatrix4(result, &node->mTransformation);
	} else {
		*result = node->mTransformation;
	}
}

void transformmesh(struct aiScene *scene, struct mesh *mesh)
{
	struct aiMesh *amesh = mesh->mesh;
	aiMatrix4x4 skin4;
	aiMatrix3x3 skin3;
	int i, k;

	if (amesh->mNumBones == 0)
		return;

	memset(mesh->position, 0, mesh->vertexcount * 3 * sizeof(float));
	memset(mesh->normal, 0, mesh->vertexcount * 3 * sizeof(float));

	for (k = 0; k < amesh->mNumBones; k++) {
		struct aiBone *bone = amesh->mBones[k];
		struct aiNode *node = findnode(scene->mRootNode, bone->mName.data);

		transformnode(&skin4, node);
		aiMultiplyMatrix4(&skin4, &bone->mOffsetMatrix);
		extract3x3(&skin3, &skin4);

		for (i = 0; i < bone->mNumWeights; i++) {
			int v = bone->mWeights[i].mVertexId;
			float w = bone->mWeights[i].mWeight;

			aiVector3D position = amesh->mVertices[v];
			aiTransformVecByMatrix4(&position, &skin4);
			mesh->position[v*3+0] += position.x * w;
			mesh->position[v*3+1] += position.y * w;
			mesh->position[v*3+2] += position.z * w;

			aiVector3D normal = amesh->mNormals[v];
			aiTransformVecByMatrix3(&normal, &skin3);
			mesh->normal[v*3+0] += normal.x * w;
			mesh->normal[v*3+1] += normal.y * w;
			mesh->normal[v*3+2] += normal.z * w;
		}
	}
}

unsigned int loadMask(){
	unsigned int texture;
	unsigned int color;
	unsigned char *c = (unsigned char*)&color;
	unsigned char a,r,g,b;

	a = 0xff;
	r = curMesh * 64;
	g = curMesh * 32;
	b = 128 + curMesh * 16 + curMesh;

	c[0] = b;
	c[1] = r;
	c[2] = g;
	c[3] = a;

	unsigned int colors[256*256];
	for(int i=0;i<256*256;i++){
		colors[i] = color;
	}

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, colors);
	//glGenerateMipmap(GL_TEXTURE_2D);

	return texture;
}

void initmesh(struct aiScene *scene, struct mesh *mesh, struct aiMesh *amesh)
{
	int i;

	vertexcount += amesh->mNumVertices;
	facecount += amesh->mNumFaces;

	mesh->mesh = amesh; // stow away pointer for bones


	curMesh++;
	mesh->texture = loadmaterial(scene->mMaterials[amesh->mMaterialIndex]);
	mesh->maskTexture = loadMask();

	mesh->vertexcount = amesh->mNumVertices;
	mesh->position = (float*)calloc(mesh->vertexcount * 3, sizeof(float));
	mesh->normal = (float*)calloc(mesh->vertexcount * 3, sizeof(float));
	mesh->texcoord = (float*)calloc(mesh->vertexcount * 2, sizeof(float));

	for (i = 0; i < mesh->vertexcount; i++) {
		mesh->position[i*3+0] = amesh->mVertices[i].x;
		mesh->position[i*3+1] = amesh->mVertices[i].y;
		mesh->position[i*3+2] = amesh->mVertices[i].z;

		if (amesh->mNormals) {
			mesh->normal[i*3+0] = amesh->mNormals[i].x;
			mesh->normal[i*3+1] = amesh->mNormals[i].y;
			mesh->normal[i*3+2] = amesh->mNormals[i].z;
		}

		if (amesh->mTextureCoords[0]) {
			mesh->texcoord[i*2+0] = amesh->mTextureCoords[0][i].x;
			mesh->texcoord[i*2+1] = 1 - amesh->mTextureCoords[0][i].y;
		}
	}

	mesh->elementcount = amesh->mNumFaces * 3;
	mesh->element = (int*)calloc(mesh->elementcount, sizeof(int));

	for (i = 0; i < amesh->mNumFaces; i++) {
		struct aiFace *face = amesh->mFaces + i;
		mesh->element[i*3+0] = face->mIndices[0];
		mesh->element[i*3+1] = face->mIndices[1];
		mesh->element[i*3+2] = face->mIndices[2];
	}

	
}

void initscene(struct aiScene *scene)
{
	int i;
	meshcount = scene->mNumMeshes;
	meshlist = (mesh*)calloc(meshcount, sizeof *meshlist);
	for (i = 0; i < meshcount; i++) {
		initmesh(scene, meshlist + i, scene->mMeshes[i]);
		transformmesh(scene, meshlist + i);
	}
}

void drawmesh(struct mesh *mesh, bool mask){
	if(mask){
		glColor4f(1, 1, 1, 1);
		glBindTexture(GL_TEXTURE_2D, mesh->maskTexture);
	}
	else{
		if (mesh->texture > 0) {
			glColor4f(1, 1, 1, 1);
			glBindTexture(GL_TEXTURE_2D, mesh->texture);
		} else {
			glColor4f(0.9, 0.7, 0.7, 1);
			glBindTexture(GL_TEXTURE_2D, checker_texture);
		}
	}
	glVertexPointer(3, GL_FLOAT, 0, mesh->position);
	glNormalPointer(GL_FLOAT, 0, mesh->normal);
	glTexCoordPointer(2, GL_FLOAT, 0, mesh->texcoord);
	glDrawElements(GL_TRIANGLES, mesh->elementcount, GL_UNSIGNED_INT, mesh->element);
}

Mat genMask(Mat m1,Mat m2){
	Mat mask(screenh,screenw,CV_8UC4);
	for(int i=0;i<m1.rows;i++){
		unsigned int *ptr = m1.ptr<unsigned int>(i);
		unsigned int *ptr2 = m2.ptr<unsigned int>(i);
		unsigned int *mptr = mask.ptr<unsigned int>(i);
		for(int j=0;j<m1.cols;j++){
			if(ptr[j] != ptr2[j]){
				mptr[j] = 0xFFFF0000;
			}
			else if((ptr2[j] >> 24) & 0xff != 0){
				mptr[j] = 0xFF00FF00;
			}
			else{
				mptr[j] = 0;
			}
		}
	}
	return mask;
}

void drawnode(struct aiNode *node, aiMatrix4x4 world)
{
	float mat[16];
	int i;

	aiMultiplyMatrix4((aiMatrix4x4*)&world, &node->mTransformation);
	transposematrix(mat, &world);
	/*

	for (i = 0; i < node->mNumMeshes; i++) {
		struct mesh *mesh = meshlist + node->mMeshes[i];
		if (mesh->mesh->mNumBones == 0) {
			// non-skinned meshes are in node-local space
			glPushMatrix();
			glMultMatrixf(mat);
			if(rand() % 2)
				drawmesh(mesh);
			glPopMatrix();
		} else {
			// skinned meshes are already in world space
			drawmesh(mesh);
			
			
		}
		
	}

	for (i = 0; i < node->mNumChildren; i++)
		drawnode(node->mChildren[i], world);

	*/
	for(int i=0;i<meshcount;i++){
		drawmesh(&meshlist[i],drawMask);
	}
	/*
	if(generate){
		Mat ss = screenshot();
		std::string filename = savedir + "img/" + prefix + std::to_string(imgcount) + ".png";

		

		//core->tPool->enqueuev([&filename,&ss](){
		//imwrite(filename,ss);
		drawmesh(&meshlist[targetmesh],false);
		Mat ss2 = screenshot();
		//filename = savedir + prefix + std::to_string(imgcount) + ".png";

		//imgcount++;

		
		Mat mask = genMask(ss,ss2);
		core->tPool->enqueuev([filename,ss2](){
			imwrite(filename,ss2);
		});
		filename = savedir + "mask/" + prefix + std::to_string(imgcount) + "mask" + ".png";
		//imwrite(filename,mask);
		core->tPool->enqueuev([filename,mask](){
			imwrite(filename,mask);
		});
		imgcount++;

	}
	*/

}

void drawscene(struct aiScene *scene)
{
	aiMatrix4x4 world;

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	aiIdentityMatrix4((aiMatrix4x4*)&world);
	drawnode(scene->mRootNode, world);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

void measuremesh(struct mesh *mesh, aiMatrix4x4 transform, float bboxmin[3], float bboxmax[3])
{
	aiVector3D p;
	int i;
	for (i = 0; i < mesh->vertexcount; i++) {
		p.x = mesh->position[i*3+0];
		p.y = mesh->position[i*3+1];
		p.z = mesh->position[i*3+2];
		aiTransformVecByMatrix4(&p, &transform);
		bboxmin[0] = MIN(bboxmin[0], p.x);
		bboxmin[1] = MIN(bboxmin[1], p.y);
		bboxmin[2] = MIN(bboxmin[2], p.z);
		bboxmax[0] = MAX(bboxmax[0], p.x);
		bboxmax[1] = MAX(bboxmax[1], p.y);
		bboxmax[2] = MAX(bboxmax[2], p.z);
	}
}

void measurenode(struct aiNode *node, aiMatrix4x4 world, float bboxmin[3], float bboxmax[3])
{
	aiMatrix4x4 identity;
	int i;

	aiMultiplyMatrix4((aiMatrix4x4*)&world, &node->mTransformation);
	aiIdentityMatrix4(&identity);

	for (i = 0; i < node->mNumMeshes; i++) {
		struct mesh *mesh = meshlist + node->mMeshes[i];
		if (mesh->mesh->mNumBones == 0) {
			// non-skinned meshes are in node-local space
			measuremesh(mesh, world, bboxmin, bboxmax);
		} else {
			// skinned meshes are already in world space
			measuremesh(mesh, identity, bboxmin, bboxmax);
		}
	}

	for (i = 0; i < node->mNumChildren; i++)
		measurenode(node->mChildren[i], world, bboxmin, bboxmax);
}

float measurescene(struct aiScene *scene, float center[3])
{
	aiMatrix4x4 world;
	float bboxmin[3];
	float bboxmax[3];
	float dx, dy, dz;

	bboxmin[0] = 1e10; bboxmax[0] = -1e10;
	bboxmin[1] = 1e10; bboxmax[1] = -1e10;
	bboxmin[2] = 1e10; bboxmax[2] = -1e10;

	aiIdentityMatrix4((aiMatrix4x4*)&world);
	measurenode(scene->mRootNode, world, bboxmin, bboxmax);

	center[0] = (bboxmin[0] + bboxmax[0]) / 2;
	center[1] = (bboxmin[1] + bboxmax[1]) / 2;
	center[2] = (bboxmin[2] + bboxmax[2]) / 2;

	dx = MAX(center[0] - bboxmin[0], bboxmax[0] - center[0]);
	dy = MAX(center[1] - bboxmin[1], bboxmax[1] - center[1]);
	dz = MAX(center[2] - bboxmin[2], bboxmax[2] - center[2]);

	return sqrt(dx*dx + dy*dy + dz*dz);
}

int animationlength(struct aiAnimation *anim)
{
	int i, len = 0;
	for (i = 0; i < anim->mNumChannels; i++) {
		struct aiNodeAnim *chan = anim->mChannels[i];
		len = MAX(len, chan->mNumPositionKeys);
		len = MAX(len, chan->mNumRotationKeys);
		len = MAX(len, chan->mNumScalingKeys);
	}
	return len;
}

void animatescene(struct aiScene *scene, struct aiAnimation *anim, float tick)
{
	struct aiVectorKey *p0, *p1, *s0, *s1;
	struct aiQuatKey *r0, *r1;
	aiVector3D p, s;
	aiQuaternion r;
	int i;

	// Assumes even key frame rate and synchronized pos/rot/scale keys.
	// We should look at the key->mTime values instead, but I'm lazy.

	int frame = floor(tick);
	float t = tick - floor(tick);

	for (i = 0; i < anim->mNumChannels; i++) {
		struct aiNodeAnim *chan = anim->mChannels[i];
		struct aiNode *node = findnode(scene->mRootNode, chan->mNodeName.data);
		p0 = chan->mPositionKeys + (frame+0) % chan->mNumPositionKeys;
		p1 = chan->mPositionKeys + (frame+1) % chan->mNumPositionKeys;
		r0 = chan->mRotationKeys + (frame+0) % chan->mNumRotationKeys;
		r1 = chan->mRotationKeys + (frame+1) % chan->mNumRotationKeys;
		s0 = chan->mScalingKeys + (frame+0) % chan->mNumScalingKeys;
		s1 = chan->mScalingKeys + (frame+1) % chan->mNumScalingKeys;
		mixvector(&p, &p0->mValue, &p1->mValue, t);
		mixquaternion(&r,&r0->mValue,&r1->mValue, t);
		mixvector(&s, &s0->mValue, &s1->mValue, t);
		composematrix(&node->mTransformation, &p, &r, &s);
	}

	for (i = 0; i < meshcount; i++)
		transformmesh(scene, meshlist + i);
}

/*
 * Boring UI and GLUT hooks.
 */

#include "getopt.c"

#define ISOMETRIC 35.264	// true isometric view
#define DIMETRIC 30		// 2:1 'isometric' as seen in pixel art



struct {
	float distance;
	float yaw;
	float pitch;
	float center[3];
} camera = { 10, 45, -DIMETRIC, { 0, 1, 0 } };

void setanim(int i)
{
	if (!g_scene) return;
	if (g_scene->mNumAnimations == 0) return;
	i = MIN(i, g_scene->mNumAnimations - 1);
	curanim = g_scene->mAnimations[i];
	animlen = animationlength(curanim);
	animfps = 30;
	animtick = 0;
	if (animfps < 1)
		animfps = 30;
}

void perspective(float fov, float aspect, float znear, float zfar)
{
	fov = fov * 3.14159 / 360.0;
	fov = tan(fov) * znear;
	glFrustum(-fov * aspect, fov * aspect, -fov, fov, znear, zfar);
}

void orthogonal(float fov, float aspect, float znear, float zfar)
{
	glOrtho(-fov * aspect, fov * aspect, -fov, fov, znear, zfar);
}

void drawstring(float x, float y, const char *s)
{
	glRasterPos2f(x+0.375, y+0.375);
	while (*s)
		//glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *s++);
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *s++);
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON) mouseleft = state == GLUT_DOWN;
	if (button == GLUT_MIDDLE_BUTTON) mousemiddle = state == GLUT_DOWN;
	if (button == GLUT_RIGHT_BUTTON) mouseright = state == GLUT_DOWN;
	mousex = x;
	mousey = y;
}

void motion(int x, int y)
{
	int dx = x - mousex;
	int dy = y - mousey;
	if (mouseleft) {
		camera.yaw -= dx * 0.3;
		camera.pitch -= dy * 0.2;
		if (camera.pitch < -85) camera.pitch = -85;
		if (camera.pitch > 85) camera.pitch = 85;
		if (camera.yaw < 0) camera.yaw += 360;
		if (camera.yaw > 360) camera.yaw -= 360;
	}
	if (mousemiddle || mouseright) {
		camera.distance += dy * 0.01 * camera.distance;
		if (camera.distance < mindist) camera.distance = mindist;
		if (camera.distance > maxdist) camera.distance = maxdist;
	}
	mousex = x;
	mousey = y;
	glutPostRedisplay();
}

void togglefullscreen(void)
{
	static int oldw = 100, oldh = 100;
	static int oldx = 0, oldy = 0;
	static int isfullscreen = 0;
	if (!isfullscreen) {
		oldw = screenw;
		oldh = screenh;
		oldx = glutGet(GLUT_WINDOW_X);
		oldy = glutGet(GLUT_WINDOW_Y);
		glutFullScreen();
	} else {
		glutPositionWindow(oldx, oldy);
		glutReshapeWindow(oldw, oldh);
	}
	isfullscreen = !isfullscreen;
}

void keyboard(unsigned char key, int x, int y)
{
	static int curanimm = 0;
	switch (key) {
	case 27: case 'q': exit(1); break;
	case 'h': case '?': showhelp = !showhelp; break;
	case 'f': togglefullscreen(); break;
	case 'i': doperspective = 0; camera.yaw = 45; camera.pitch = -DIMETRIC; break;
	case 'I': doperspective = 0; camera.yaw = 45; camera.pitch = -ISOMETRIC; break;
	case 'p': doperspective = !doperspective; break;
	case '0': animtick = 0; animfps = 30; break;
	case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8':
	case '9': setanim(key - '1'); break;
	case ' ': playing = !playing; break;
	case '.': animtick = floor(animtick) + 1; break;
	case ',': animtick = floor(animtick) - 1; break;
	case '[': animfps = MAX(5, animfps-5); break;
	case ']': animfps = MIN(60, animfps+5); break;
	case 'g': doplane = !doplane; break;
	case 't': dotexture = !dotexture; break;
	case 'A': doalpha--; break;
	case 'a': doalpha++; break;
	case 'w': dowire = !dowire; break;
	case 'b': dobackface = !dobackface; break;
	case 'o':
		setanim(curanimm++);
		break;
	case 'l':
		setanim(curanimm--);
		break;
	}

	if (playing)
		lasttime = glutGet(GLUT_ELAPSED_TIME);

	glutPostRedisplay();
}

void special(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_F4: exit(1); break;
	case GLUT_KEY_F1: showhelp = !showhelp; break;
	}
	glutPostRedisplay();
}

void reshape(int w, int h)
{

}

float float_rand( float min, float max )
{
    float scale = (float)rand() / (float) RAND_MAX; /* [0, 1.0] */
    return min + scale * ( max - min );      /* [min, max] */
}





void display(void){
	if(imgcount >= maxImgCount){
		std::this_thread::sleep_for(std::chrono::milliseconds(20000));
		glutLeaveMainLoop();
		return;
	}
	char buf[256];
	int time, timestep;
	int i;


	glViewport(0, 0, screenw, screenh);

	drawMask = false;


	time = glutGet(GLUT_ELAPSED_TIME);
	timestep = time - lasttime;
	lasttime = time;

	if(generate){
		camera.pitch = rand() % 120 - 80;
		camera.yaw = rand() % 360;
	}

	if (camera.pitch < -50) camera.pitch = -50;
	if (camera.pitch > 50) camera.pitch = 50;
	if (camera.yaw < 0) camera.yaw += 360;
	if (camera.yaw > 360) camera.yaw -= 360;


	if (g_scene) {
		if (curanim) {
			if (playing) {
				animtick = animtick + (timestep/1000.0) * animfps;
			}
			if(generate){
				setanim(rand() % g_scene->mNumAnimations);
				animtick = rand() % animlen;
			}
			animatescene(g_scene, curanim, animtick);
		}
	}

	float radi = measurescene(g_scene,camera.center);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawPixels(screenw,screenh,GL_BGRA,GL_UNSIGNED_BYTE,zeros);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (doperspective)
		perspective(50, (float)screenw/screenh, mindist/5, maxdist*5);
	else
		orthogonal(camera.distance/2, (float)screenw/screenh, mindist/5, maxdist*5);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	if(generate){
		light_position[0] = float_rand(-2,2);
		light_position[1] = float_rand(-2,2);
		light_position[2] = float_rand(-2,2);
	}
	glEnable(GL_LIGHTING);

	glEnable(GL_LIGHT0);
	//float light_position[4] = { -1, 2, 2, 0 };
	
	float aLight;
	if(generate)
		aLight = float_rand(0.3,0.5);
	else
		aLight = 1.0;
	
	float a_light_position[4] = { aLight, aLight, aLight, 0 };


	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	if(generate){
		for(int i=0;i<1;i++){
			glEnable(GL_LIGHT1+i);
			light_position[0] = float_rand(radi * -3 ,radi * 3);
			light_position[1] = float_rand(radi * -3 ,radi * 3);
			light_position[2] = float_rand(radi * -3 ,radi * 3);
			glLightfv(GL_LIGHT1+i, GL_POSITION, light_position);
		}
	}
	
	
	glEnable(GL_LIGHT7);
	glLightfv(GL_LIGHT7,GL_AMBIENT,a_light_position);

	glTranslatef(0, 0, -camera.distance);
	glRotatef(-camera.pitch, 1, 0, 0);
	glRotatef(-camera.yaw, 0, 1, 0);
	glTranslatef(-camera.center[0], -camera.center[1], -camera.center[2]);

	
	glEnable(GL_TEXTURE_2D);



	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (dobackface)
		glDisable(GL_CULL_FACE);
	else
		glEnable(GL_CULL_FACE);

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, dotwosided);


	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_EQUAL, 1);
	
	if (g_scene) drawscene(g_scene);
	


	

	glAlphaFunc(GL_LESS, 1);
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);


	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_TEXTURE_2D);

	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);

	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, screenw, screenh, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	//Mask

	if(generate){
		Mat ss = screenshot();
		std::string filename = savedir + "img/" + prefix + std::to_string(imgcount) + ".png";

		

		
	
	

		drawMask = true;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawPixels(screenw,screenh,GL_BGRA,GL_UNSIGNED_BYTE,zeros);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		if (doperspective)
			perspective(50, (float)screenw/screenh, mindist/5, maxdist*5);
		else
			orthogonal(camera.distance/2, (float)screenw/screenh, mindist/5, maxdist*5);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_COLOR_MATERIAL);

		glEnable(GL_LIGHTING);

		glEnable(GL_LIGHT0);
		//float light_position[4] = { -1, 2, 2, 0 };
		

		aLight = 1.0;
		
		float am_light_position[4] = { aLight, aLight, aLight, 0 };


		
		
		glEnable(GL_LIGHT0);
		glLightfv(GL_LIGHT0,GL_AMBIENT,am_light_position);

		glTranslatef(0, 0, -camera.distance);
		glRotatef(-camera.pitch, 1, 0, 0);
		glRotatef(-camera.yaw, 0, 1, 0);
		glTranslatef(-camera.center[0], -camera.center[1], -camera.center[2]);

		
		glEnable(GL_TEXTURE_2D);



		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (dobackface)
			glDisable(GL_CULL_FACE);
		else
			glEnable(GL_CULL_FACE);

		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, dotwosided);


		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_EQUAL, 1);


		if (g_scene) drawscene(g_scene);

		glAlphaFunc(GL_LESS, 1);
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);


		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_TEXTURE_2D);

		glDisable(GL_LIGHTING);
		glDisable(GL_COLOR_MATERIAL);

		glDisable(GL_DEPTH_TEST);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, screenw, screenh, 0, -1, 1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		std::string mfilename = savedir + "mask/" + prefix + std::to_string(imgcount) + "mask" + ".png";
		//imwrite(filename,mask);
		
		if(imgcount > 0){
			


			Mat mask = screenshot();
			core->tPool->enqueuev([mfilename,mask](){
				flip(mask,crop(mask),1);
				imwrite(mfilename,mask);
			});

			core->tPool->enqueuev([filename,ss](){
				imwrite(filename,crop(ss));
			});
		}
		imgcount++;
	}

	glColor4f(1, 1, 1, 1);
	if (g_scene) {
		sprintf(buf, "%d meshes; %d vertices; %d faces ", meshcount, vertexcount, facecount);
		drawstring(8, 18+0, buf);
		if (curanim) {
			sprintf(buf, "frame %03d / %03d (%d fps)", (int)animtick+1, animlen, animfps);
			drawstring(8, 18+20, buf);
			drawstring(8, 18+40, curanim->mName.C_Str());
		}
	} else {
		drawstring(8, 18, "No model loaded!");
	}

	if (showhelp) {
		#define Y(n) 18+40+n*16
		
	}


	glutSwapBuffers();

	glutPostRedisplay();

	i = glGetError();
	if (i) fprintf(stderr, "opengl error: %d\n", i);
}

void usage(void)
{
	fprintf(stderr, "usage: assview [-geometry WxH] [options] asset.dae\n");
	fprintf(stderr, "\t-i\tdimetric (2:1) camera\n");
	fprintf(stderr, "\t-I\ttrue isometric camera\n");
	fprintf(stderr, "\t-a\talpha transparency mode; use more times for higher quality.\n");
	fprintf(stderr, "\t-b\tdon't render backfaces\n");
	fprintf(stderr, "\t-g\trender ground plane\n");
	fprintf(stderr, "\t-l\tone-sided lighting\n");
	fprintf(stderr, "\t-t\tdon't render textures\n");
	fprintf(stderr, "\t-w\trender wireframe\n");
	fprintf(stderr, "\t-c r,g,b\tbackground color\n");
	fprintf(stderr, "\t-r n\trotate camera n degrees (yaw)\n");
	fprintf(stderr, "\t-p n\tpitch camera n degrees\n");
	fprintf(stderr, "\t-z n\tzoom camera n times\n");
	fprintf(stderr, "\t-f n\trender animation at frame n\n");
	exit(1);
}

int main(int argc, char **argv)
{

	if(argc < 8)
		return 1;

	srand(time(NULL));

	float clearcolor[4] = { 0.0, 0.0, 0.0, 0.0 };
	float zoom = 1;
	int c;

	core = HCore::init(16,1);
	auxCore = new HCore::HCore(16,1);

	zeros = (unsigned char*)calloc(screenw*screenh*4,sizeof(char));

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(50, 50);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE | GLUT_ALPHA);



	glutCreateWindow("Hrender");


	glewInit();

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	/* Color renderbuffer. */
	glGenRenderbuffers(1, &rbo_color);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo_color);
	/* Storage must be one of: */
	/* GL_RGBA4, GL_RGB565, GL_RGB5_A1, GL_DEPTH_COMPONENT16, GL_STENCIL_INDEX8. */
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, screenw, screenh);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo_color);

	/* Depth renderbuffer. */
	glGenRenderbuffers(1, &rbo_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, screenw, screenh);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth);

	glReadBuffer(GL_COLOR_ATTACHMENT0);

	glClearColor(0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glViewport(0, 0, screenw, screenh);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);



	initchecker();
	prefix = argv[2];
	savedir = argv[3];
	maxImgCount = atoi(argv[4]);
	generate = atoi(argv[5]);
	targetmesh = atoi(argv[7]);
	
	int flags = aiProcess_Triangulate;
	flags |= aiProcess_JoinIdenticalVertices;
	flags |= aiProcess_GenSmoothNormals;
	flags |= aiProcess_GenUVCoords;
	flags |= aiProcess_TransformUVCoords;
	flags |= aiProcess_RemoveComponent;

	strcpy(basedir, argv[1]);
	char *p = strrchr(basedir, '/');
	if (!p) p = strrchr(basedir, '\\');
	if (!p) strcpy(basedir, ""); else p[1] = 0;

	glutSetWindowTitle(argv[1]);

	g_scene = (struct aiScene*) aiImportFile(argv[1], flags);
	if (g_scene) {
		initscene(g_scene);

		float radius = measurescene(g_scene, camera.center);
		gridsize = (int)radius + 1;
		mindist = radius * 0.1;
		maxdist = radius * 10;
		camera.distance = atoi(argv[6]) * radius;

		setanim(0);
	} else {
		fprintf(stderr, "cannot import scene: '%s'\n", argv[1]);
	}


	glEnable(GL_MULTISAMPLE);
	glEnable(GL_NORMALIZE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glClearColor(clearcolor[0], clearcolor[1], clearcolor[2], clearcolor[3]);

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);





	glutMainLoop();

	return 0;
}

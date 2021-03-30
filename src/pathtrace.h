#pragma once

#include <vector>
#include "scene.h"

void pathtraceInit(Scene *scene, bool octree, int treeDepth, int numGeoms, int totalIter);
void pathtraceFree(bool octree);
void filterInit(int filterSize);
void filterFree();
void pathtrace(uchar4 *pbo, int frame, int iteration, bool cacheFirstBounce, bool sortByMaterial, bool useMeshBounds);
void denoiseImage(int num_iters, float c_phi, float n_phi, float p_phi);
std::vector<glm::vec3> getDenoisedImage();
void showGBuffer(uchar4* pbo, int buffer_type);
void showImage(uchar4* pbo, int iter, bool denoised);

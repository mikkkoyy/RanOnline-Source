#include "stdafx.h"

#include <algorithm>
#include "../Common/StlFunctions.h"

#include "../G-Logic/glperiod.h"
#include "../Common/SerialFile.h"
#include "DxLightMan.h"
#include "./TextureManager.h"

#include "NsOptimizeMesh.h"
#include "../DxEffect/DxTextureEffMan.h"

#include "../DxEffect/DxTexEffMan.h"
#include "../DxEffect/DxTexEffDiffuse.h"
#include "../DxEffect/DxTexEffFlowUV.h"
#include "../DxEffect/DxTexEffRotate.h"
#include "../DxEffect/DxTexEffSpecular.h"

#include "NsOCTree.h"

#ifdef _DEBUG                                                                                                                                                                                                 
#define new DEBUG_NEW
#endif

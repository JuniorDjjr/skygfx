
#include "skygfx.h"
#include "d3d9helper.h"
#include <DirectXMath.h>

typedef D3DMATRIX D3DXMATRIX;

D3DMATRIX &_RwD3D9D3D9ViewTransform = *(D3DMATRIX*)0xC9BC80;
D3DMATRIX &_RwD3D9D3D9ProjTransform = *(D3DMATRIX*)0x8E2458;
D3DMATRIX &D3D9ViewProjTransform = *(D3DMATRIX*)0xC94C30;
D3DMATRIX &D3D9Transform = *(D3DMATRIX*)0x8D7390;
D3DMATRIX *&_RwD3D9ActiveViewProjTransform = *(D3DMATRIX**)0xC97C64;
RwMatrix *&D3D9ActiveTransform = *(RwMatrix**)0xC94C70;
D3DMATRIX D3D9WorldViewTransform;	/* not in RW */

WRAPPER D3DXMATRIX *__stdcall D3DXMatrixMultiply(D3DXMATRIX*, const D3DXMATRIX*, const D3DXMATRIX*) { EAXJMP(0x7672E1); }
WRAPPER D3DXMATRIX *__stdcall D3DXMatrixMultiplyTranspose(D3DXMATRIX*, const D3DXMATRIX*, const D3DXMATRIX*) { EAXJMP(0x7676C7); }
WRAPPER D3DXMATRIX *__stdcall D3DXMatrixTranspose(D3DXMATRIX*, const D3DXMATRIX*) { EAXJMP(0x7675EE); }

//int __stdcall D3DXCompileShaderFromFileA(LPCSTR lpFileName, int a2, int a3, const char *a4, const char *a5, unsigned int a6, struct ID3DXBuffer **a7, int a8, struct ID3DXConstantTable **a9)
//v1 = D3DXCompileShaderFromFileA(FileName, 0, 0, "main", "ps_2_0", 1, &shader, &errors, v8);
WRAPPER int D3DXCompileShaderFromFileA(LPCSTR lpFileName, int a2, int a3, const char* a4, const char* a5, unsigned int a6, RwUInt32* shader, int* errors, struct ID3DXConstantTable** a9) { EAXJMP(0x7FAC60); }

/* Fixed RW function */
void
_rwD3D9VSGetComposedTransformMatrix(void *transformMatrix)
{
	/* Probably not that useful anymore */
	if(_RwD3D9ActiveViewProjTransform == NULL){
		D3DXMatrixMultiply(&D3D9ViewProjTransform, &_RwD3D9D3D9ViewTransform, &_RwD3D9D3D9ProjTransform);
		_RwD3D9ActiveViewProjTransform = &D3D9ViewProjTransform;
	}

	if(D3D9ActiveTransform != NULL){
		/* W * (V * P) is bad. we want (W * V) * P */
		//D3DXMatrixMultiplyTranspose((D3DMATRIX*)transformMatrix, &D3D9Transform, _RwD3D9ActiveViewProjTransform);
		D3DXMatrixMultiply(&D3D9WorldViewTransform, &D3D9Transform, &_RwD3D9D3D9ViewTransform);
		D3DXMatrixMultiplyTranspose((D3DMATRIX*)transformMatrix, &D3D9WorldViewTransform, &_RwD3D9D3D9ProjTransform);
	}else
		D3DXMatrixTranspose((D3DMATRIX*)transformMatrix, _RwD3D9ActiveViewProjTransform);
}

void
RwToD3DMatrix(void *d3d, RwMatrix *rw)
{
	D3DMATRIX *m = (D3DMATRIX*)d3d;
	m->m[0][0] = rw->right.x;
	m->m[0][1] = rw->up.x;
	m->m[0][2] = rw->at.x;
	m->m[0][3] = rw->pos.x;
	m->m[1][0] = rw->right.y;
	m->m[1][1] = rw->up.y;
	m->m[1][2] = rw->at.y;
	m->m[1][3] = rw->pos.y;
	m->m[2][0] = rw->right.z;
	m->m[2][1] = rw->up.z;
	m->m[2][2] = rw->at.z;
	m->m[2][3] = rw->pos.z;
	m->m[3][0] = 0.0f;
	m->m[3][1] = 0.0f;
	m->m[3][2] = 0.0f;
	m->m[3][3] = 1.0f;
}


void
MakeProjectionMatrix(void *d3d, RwCamera *cam, float nbias, float fbias)
{
	float f = cam->farPlane + fbias;
	float n = cam->nearPlane + nbias;
	D3DMATRIX *m = (D3DMATRIX*)d3d;
	m->m[0][0] = cam->recipViewWindow.x;
	m->m[0][1] = 0.0f;
	m->m[0][2] = 0.0f;
	m->m[0][3] = 0.0f;
	m->m[1][0] = 0.0f;
	m->m[1][1] = cam->recipViewWindow.y;
	m->m[1][2] = 0.0f;
	m->m[1][3] = 0.0f;
	m->m[2][0] = 0.0f;
	m->m[2][1] = 0.0f;
	m->m[2][2] = f/(f-n);
	m->m[2][3] = -n*m->m[2][2];
	m->m[3][0] = 0.0f;
	m->m[3][1] = 0.0f;
	m->m[3][2] = 1.0f;
	m->m[3][3] = 0.0f;
}

enum {
	LOC_combined = 0,
	LOC_world = 1,
};
void
transpose(void *dst, void *src)
{
	float (*m1)[4] = (float(*)[4])dst;
	float (*m2)[4] = (float(*)[4])src;
	m1[0][0] = m2[0][0];
	m1[0][1] = m2[1][0];
	m1[0][2] = m2[2][0];
	m1[0][3] = m2[3][0];
	m1[1][0] = m2[0][1];
	m1[1][1] = m2[1][1];
	m1[1][2] = m2[2][1];
	m1[1][3] = m2[3][1];
	m1[2][0] = m2[0][2];
	m1[2][1] = m2[1][2];
	m1[2][2] = m2[2][2];
	m1[2][3] = m2[3][2];
	m1[3][0] = m2[0][3];
	m1[3][1] = m2[1][3];
	m1[3][2] = m2[2][3];
	m1[3][3] = m2[3][3];
}

static DirectX::XMMATRIX pipeWorldMat, pipeViewMat, pipeProjMat;

void
pipeGetComposedTransformMatrix(RpAtomic *atomic, float *out)
{
	RwMatrix *world = RwFrameGetLTM(RpAtomicGetFrame(atomic));

	RwToD3DMatrix(&pipeWorldMat, world);
	transpose(&pipeViewMat, &_RwD3D9D3D9ViewTransform);
	transpose(&pipeProjMat, &_RwD3D9D3D9ProjTransform);

	DirectX::XMMATRIX combined = DirectX::XMMatrixMultiply(pipeProjMat, DirectX::XMMatrixMultiply(pipeViewMat, pipeWorldMat));
	memcpy(out, &combined, 64);
}

// Have to call the above before!
void
pipeGetCameraTransformMatrix(float *out)
{
	DirectX::XMMATRIX combined = DirectX::XMMatrixMultiply(pipeViewMat, pipeWorldMat);
	memcpy(out, &combined, 64);
}

// Have to call the above before!
void
pipeGetWorldMatrix(float *out)
{
	memcpy(out, &pipeWorldMat, 64);
}

void
pipeGetLeedsEnvMapMatrix(RpAtomic *atomic, float *out)
{
	DirectX::XMMATRIX tmp;

	RwMatrix *world = RwFrameGetLTM(RpAtomicGetFrame(atomic));
	RwCamera *cam = (RwCamera*)RWSRCGLOBAL(curCamera);

	float view[16];
	// Kill pitch in camera matrix
	RwMatrix mat = *RwFrameGetLTM(RwCameraGetFrame(cam));
	mat.pos.x = 0.0f;
	mat.pos.y = 0.0f;
	mat.pos.z = 0.0f;
	mat.at.z = 0.0f;
	RwV3dNormalize(&mat.at, &mat.at);
	mat.up.x = 0.0f;
	mat.up.y = 0.0f;
	mat.up.z = 1.0f;
	mat.right.x = -mat.at.y;
	mat.right.y = mat.at.x;
	mat.right.z = 0;
	mat.flags = rwMATRIXTYPEORTHONORMAL;

	// this can be simplified, invert is just transpose as well
	RwMatrixInvert((RwMatrix*)view, &mat);
	view[0] = -view[0];
	view[3] = 0.0f;
	view[4] = -view[4];
	view[7] = 0.0f;
	view[8] = -view[8];
	view[11] = 0.0f;
	view[12] = -view[12];
	view[15] = 1.0f;
	transpose(&tmp, view);

	tmp = DirectX::XMMatrixMultiply(tmp, pipeWorldMat);
	memcpy(out, &tmp, 64);
}

void
pipeUploadMatCol(int flags, RpMaterial *m, int loc)
{
	static float white[4] = { 1.0, 1.0, 1.0, 1.0 };
	if(flags & rpGEOMETRYMODULATEMATERIALCOLOR){
		RwRGBAReal color;
		RwRGBARealFromRwRGBA(&color, &m->color);
		RwD3D9SetVertexShaderConstant(loc, &color, 1);
	}else
		RwD3D9SetVertexShaderConstant(loc, white, 1);
}

void
pipeUploadZero(int loc)
{
	static float z[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	RwD3D9SetVertexShaderConstant(loc, (void*)z, 1);
}

void
pipeUploadZeroPS(int loc)
{
	static float z[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	RwD3D9SetPixelShaderConstant(loc, (void*)z, 1);
}

void
pipeUploadLightColor(RpLight *light, int loc)
{
	float c[4];
	if(RpLightGetFlags(light) & rpLIGHTLIGHTATOMICS){
		c[0] = light->color.red;
		c[1] = light->color.green;
		c[2] = light->color.blue;
		c[3] = 1.0f;
		RwD3D9SetVertexShaderConstant(loc, (void*)c, 1);
	}else
		pipeUploadZero(loc);
}

void
pipeUploadLightColorPS(RpLight *light, int loc)
{
	float c[4];
	if(RpLightGetFlags(light) & rpLIGHTLIGHTATOMICS){
		c[0] = light->color.red;
		c[1] = light->color.green;
		c[2] = light->color.blue;
		c[3] = 1.0f;
		RwD3D9SetPixelShaderConstant(loc, (void*)c, 1);
	}else
		pipeUploadZeroPS(loc);
}

void
pipeUploadLightDirection(RpLight *light, int loc)
{
	float c[4];
	if(RpLightGetFlags(light) & rpLIGHTLIGHTATOMICS){
		RwV3d *at = RwMatrixGetAt(RwFrameGetLTM(RpLightGetFrame(light)));
		c[0] = at->x;
		c[1] = at->y;
		c[2] = at->z;
		c[3] = 1.0f;
		RwD3D9SetVertexShaderConstant(loc, (void*)c, 1);
	}else
		pipeUploadZero(loc);
}

void
pipeUploadLightDirectionPS(RpLight *light, int loc)
{
	float c[4];
	if(RpLightGetFlags(light) & rpLIGHTLIGHTATOMICS){
		RwV3d *at = RwMatrixGetAt(RwFrameGetLTM(RpLightGetFrame(light)));
		c[0] = at->x;
		c[1] = at->y;
		c[2] = at->z;
		c[3] = 1.0f;
		RwD3D9SetPixelShaderConstant(loc, (void*)c, 1);
	}else
		pipeUploadZeroPS(loc);
}

void
pipeUploadLightDirectionLocal(RpLight *light, RwMatrix *m, int loc)
{
	float c[4];
	if(RpLightGetFlags(light) & rpLIGHTLIGHTATOMICS){
		RwV3d *at = RwMatrixGetAt(RwFrameGetLTM(RpLightGetFrame(light)));
		RwV3dTransformVector((RwV3d*)c, at, m);
		c[3] = 1.0f;
		RwD3D9SetVertexShaderConstant(loc, (void*)c, 1);
	}else
		pipeUploadZero(loc);
}

void
pipeUploadLightDirectionInv(RpLight *light, int loc)
{
	float c[4];
	RwV3d *at = RwMatrixGetAt(RwFrameGetLTM(RpLightGetFrame(light)));
	c[0] = -at->x;
	c[1] = -at->y;
	c[2] = -at->z;
	RwD3D9SetVertexShaderConstant(loc, (void*)c, 1);
}

void
makePS(int res, void **sh)
{
	if(*sh == NULL){
		HRSRC resource = FindResource(dllModule, MAKEINTRESOURCE(res), RT_RCDATA);
		RwUInt32 *shader = (RwUInt32*)LoadResource(dllModule, resource);
		RwD3D9CreatePixelShader(shader, sh);
		FreeResource(shader);
	}
}

void
makeVS(int res, void **sh)
{
	if(*sh == NULL){
		HRSRC resource = FindResource(dllModule, MAKEINTRESOURCE(res), RT_RCDATA);
		RwUInt32 *shader = (RwUInt32*)LoadResource(dllModule, resource);
		RwD3D9CreateVertexShader(shader, sh);
		FreeResource(shader);
	}
}

void
makeVSfromFile(char* fileName, void** sh)
{
	if (*sh == NULL) {
		char* path = getpath("shaders\\test.fx");
		if (path == NULL)
			return;

		RwUInt32 ppShader;

		int buildResult = D3DXCompileShaderFromFileA(path, NULL, NULL, "main", "vs_2_0", 1, &ppShader, NULL, NULL);

		if (buildResult == D3D_OK)
		{
			//int v6 = (*(int(__stdcall**)(int, void*))(*(RwUInt32*)ppShader + 12))(ppShader, &xboxBuildingWindVS);
			int v177 = (*(int(__stdcall**)(int))(*(RwUInt32*)ppShader + 12))(ppShader);
			//RwD3D9CreateVertexShader((RwUInt32 *)shaderData, sh);
			if (buildResult || *sh == NULL) {
				MessageBox(nil, "Fail RwD3D9CreateVertexShader", "Error", MB_ICONERROR | MB_OK);
			}
		}
		else {
			//xboxBuildingWindVS = nullptr;
			MessageBox(nil, "Fail D3DXCompileShaderFromFileA", "Error", MB_ICONERROR | MB_OK);
		}

	}
}

void
CreateShaders(void)
{
	// postfx
	makePS(IDR_IIITRAILSPS, &iiiTrailsPS);
	makePS(IDR_VCTRAILSPS, &vcTrailsPS);
	makePS(IDR_GRADINGPS, &gradingPS);
	makePS(IDR_CONTRASTPS, &contrastPS);
	makePS(IDR_BLURPS, &blurPS);
	makePS(IDR_RADIOSITYPS, &radiosityPS);

	makePS(IDR_SIMPLEPS, &simplePS);
	makePS(IDR_SIMPLESTOCHASTICPS, &simpleStochasticPS);

	// vehicles
	makeVS(IDR_VEHICLEVS, &vehiclePipeVS);
	makeVS(IDR_PS2CARFXVS, &ps2CarFxVS);
	makePS(IDR_PS2ENVSPECFXPS, &ps2EnvSpecFxPS);	// also building
	makeVS(IDR_SPECCARFXVS, &specCarFxVS);
	makePS(IDR_SPECCARFXPS, &specCarFxPS);
	makeVS(IDR_XBOXCARVS, &xboxCarVS);
	makeVS(IDR_LEEDSCARFXVS, &leedsCarFxVS);
	makeVS(IDR_MOBILEVEHICLEVS, &mobileVehiclePipeVS);
	makePS(IDR_MOBILEVEHICLEPS, &mobileVehiclePipePS);
	makeVS(IDR_ENVCARVS, &envCarVS);
	makePS(IDR_ENVCARPS, &envCarPS);

	// building
	makeVS(IDR_PS2BUILDINGVS, &ps2BuildingVS);
	makeVS(IDR_PS2BUILDINGFXVS, &ps2BuildingFxVS);
	makeVS(IDR_XBOXBUILDINGVS, &xboxBuildingVS);
	makePS(IDR_XBOXBUILDINGPS, &xboxBuildingPS);
	makePS(IDR_XBOXBUILDINGSTOCHASTICPS, &xboxBuildingStochasticPS);
	makePS(IDR_SIMPLEDETAILPS, &simpleDetailPS);
	makePS(IDR_SIMPLEDETAILSTOCHASTICPS, &simpleDetailStochasticPS);
	makePS(IDR_SIMPLEFOGPS, &simpleFogPS);
	makeVS(IDR_SPHEREBUILDINGVS, &sphereBuildingVS);

	makeVS(IDR_XBOXBUILDINGWINDVS, &xboxBuildingWindVS);
	makeVS(IDR_PS2BUILDINGWINDVS, &ps2BuildingWindVS);

	// custom
	//makeVSfromFile("test.fx", &xboxBuildingWindVS);
}

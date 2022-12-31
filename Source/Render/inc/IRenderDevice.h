#pragma once

#include <cstdint>
#include "Unknown.h"
#include "RenderTypes.h"

enum eRenderDeviceSelection {
    DEVICE_D3D9,
    DEVICE_HEADLESS,
    DEVICE_SOKOL
};

//Возвращает минимальное число,являющееся степенью двойки и не меньше, чем n
inline int Power2up(int n)
{
    int i=1;
    while(i<n)
        i=i<<1;
    return i;
}

class VertexBuffer {
public:
    union {
        void* buf = nullptr;
#ifdef PERIMETER_D3D9
        struct IDirect3DVertexBuffer9* d3d;
#endif
        
#ifdef PERIMETER_SOKOL
        struct SokolBuffer* sg;
#endif
    };
    bool dynamic = false;
    vertex_fmt_t fmt = 0;
    uint8_t VertexSize = 0;
    int NumberVertex = 0;
    
    VertexBuffer() = default;
    ~VertexBuffer() {
        Destroy();
    }
    NO_COPY_CONSTRUCTOR(VertexBuffer)
    
    void Destroy();
};

class IndexBuffer {
public:
    union {
        void* buf = nullptr;
#ifdef PERIMETER_D3D9
        struct IDirect3DIndexBuffer9* d3d;
#endif
#ifdef PERIMETER_SOKOL
        struct SokolBuffer* sg;
#endif
    };
    uint32_t NumberIndices = 0;
    
    IndexBuffer() = default;
    ~IndexBuffer() {
        Destroy();
    }
    NO_COPY_CONSTRUCTOR(IndexBuffer)

    void Destroy();
};

struct sTextureFormatData
{
    int rBitCount,gBitCount,bBitCount,aBitCount;
    int rBitShift,gBitShift,bBitShift,aBitShift;
    int BytePerPixel;
    uint32_t FormatID;

    sTextureFormatData& Set(int bpp,int rcount,int gcount,int bcount,int acount,int rshift,int gshift,int bshift,int ashift, uint32_t formatID=-1)
    {
        BytePerPixel=bpp;
        rBitCount=rcount; gBitCount=gcount; bBitCount=bcount; aBitCount=acount;
        rBitShift=rshift; gBitShift=gshift;	bBitShift=bshift; aBitShift=ashift;
        FormatID = formatID;
        return *this;
    }

    void Get(int &bpp,int &rcount,int &gcount,int &bcount,int &acount,int &rshift,int &gshift,int &bshift,int &ashift)
    {
        bpp=BytePerPixel;
        rcount=rBitCount; gcount=gBitCount; bcount=bBitCount; acount=aBitCount;
        rshift=rBitShift; gshift=gBitShift;	bshift=bBitShift; ashift=aBitShift;
    }
};

class cIUnkClass;
class cObjMesh;

unsigned int ColorByNormal(Vect3f n);
Vect3f NormalByColor(uint32_t d);
void BuildMipMap(int x,int y,int bpp,int bplSrc,void *pSrc,int bplDst,void *pDst,
                 int rc,int gc,int bc,int ac,int rs,int gs,int bs,int as,int Blur=0);

enum TILEMAP_DRAW
{
    TILEMAP_ALL,
    TILEMAP_ZEROPLAST,
    TILEMAP_NOZEROPLAST,
};

enum eModeRenderDevice
{
    RENDERDEVICE_MODE_WINDOW	=	1<<0,	// вывод в окно

    //d3d only
    RENDERDEVICE_MODE_RGB16		=	1<<8,	// 16 битный цвет
    RENDERDEVICE_MODE_RGB32		=	1<<9,	// 32 битный цвет
    RENDERDEVICE_MODE_COMPRESS	=	1<<10,	// компресованные текстуры
    RENDERDEVICE_MODE_VSYNC		=	1<<11,	// использовать вертикальную синхронизацию
    RENDERDEVICE_MODE_STRENCIL	=	1<<12,
    RENDERDEVICE_MODE_Z24		=	1<<13,	// 24 битный  
    RENDERDEVICE_MODE_REF		=	1<<14,
    RENDERDEVICE_MODE_MULTITHREAD = 1<<15,
    RENDERDEVICE_MODE_ALPHA		=   1<<16,  //При создании rendertarget использовать alpha 

    //Internal
    RENDERDEVICE_MODE_RETURNERROR=	1<<17, //Только для ChangeSize
    RENDERDEVICE_MODE_ONEBACKBUFFER=1<<18,
};

enum eRenderStateOption
{
    RS_ZENABLE                  = 7,    /* or TRUE/FALSE for legacy */
    RS_FILLMODE                 = 8,    /* FILL_POINT, FILL_WIREFRAME or FILL_SOLID */
    RS_ZWRITEENABLE             = 14,   /* TRUE to enable z writes */
    RS_ZFUNC                    = 23,   /* CMPFUNC */
    RS_CULLMODE                 = 22,   /* Cull mode */
    RS_ALPHAREF                 = 24,   /* FIXED */
    RS_ALPHAFUNC				= 25,	/* CMPFUNC */
    RS_DITHERENABLE             = 26,   /* TRUE to enable dithering */
    RS_ALPHABLENDENABLE         = 27,   /* TRUE to enable alpha blending */
    RS_FOGENABLE                = 28,   /* TRUE to enable fog blending */
    RS_FOGCOLOR                 = 34,   /* D3DCOLOR */
    RS_FOGSTART                 = 36,   /* Fog start (for both vertex and pixel fog) */
    RS_FOGEND                   = 37,   /* Fog end      */
    RS_STENCILENABLE            = 52,
    RS_BILINEAR					= 1024,	/* Change min/mag filter */
};

enum eFillMode
{
    FILL_POINT					= 1,
    FILL_WIREFRAME				= 2,
    FILL_SOLID					= 3,
};

enum eCMPFUNC
{
    CMP_NEVER					= 1,
    CMP_LESS					= 2,
    CMP_EQUAL					= 3,
    CMP_LESSEQUAL				= 4,
    CMP_GREATER					= 5,
    CMP_NOTEQUAL				= 6,
    CMP_GREATEREQUAL			= 7,
    CMP_ALWAYS					= 8,
};

enum eColorMode
{
    COLOR_MOD,
    COLOR_ADD,
    COLOR_MOD2,
    COLOR_MOD4,
};
enum eBlendMode
{
    ALPHA_NONE,
    ALPHA_TEST,
    ALPHA_ADDBLENDALPHA,
    ALPHA_BLEND,
    ALPHA_ADDBLEND,
    ALPHA_SUBBLEND,
    ALPHA_MUL,
};

enum eCullMode
{
    CULL_NONE,
    CULL_CW,
    CULL_CCW
};

struct sPolygon {
    static const size_t PN = 3;
    indices_t p1, p2, p3;
    inline void set(indices_t i1,indices_t i2,indices_t i3) { p1=i1; p2=i2; p3=i3; }
};

class cFont;
class cTexture;
class cTexLibrary;
class cCamera;

class cInterfaceRenderDevice : public cUnknownClass
{
protected:
    cTexLibrary* TexLibrary = nullptr;
    cFont* CurrentFont = nullptr;
    cFont* DefaultFont = nullptr;
    cCamera *DrawNode = nullptr;
    Vect2i ScreenSize = { 0, 0 };
    uint32_t RenderMode = 0;
    class DrawBuffer* lastDrawBuffer = nullptr;
    std::vector<class DrawBuffer*> drawBuffers;

public:
    cInterfaceRenderDevice();
    ~cInterfaceRenderDevice() override;
    
    // Common methods

    cTexLibrary* GetTexLibrary() { return TexLibrary; }
    cTexture* GetTexture(int n);

    eModeRenderDevice GetRenderMode() { return static_cast<eModeRenderDevice>(RenderMode); }

    virtual bool IsFullScreen() { return (RenderMode&RENDERDEVICE_MODE_WINDOW) == 0; }
    
    virtual int GetSizeX() { return ScreenSize.x; };
    virtual int GetSizeY() { return ScreenSize.y; };

    inline cCamera* GetDrawNode() { return DrawNode; }
    virtual void SetDrawNode(cCamera* node) { DrawNode = node; };
    virtual void SetWorldMatXf(const MatXf& matrix);

    virtual void SetFont(cFont *pFont);
    virtual void SetDefaultFont(cFont *pFont);
    virtual float GetFontLength(const char *string, size_t* count = nullptr);
    virtual float GetCharLength(const char c);

    virtual size_t GetSizeFromFormat(vertex_fmt_t fmt) const;
    
    virtual class DrawBuffer* GetDrawBuffer(vertex_fmt_t fmt);

    // Decl only methods

    virtual eRenderDeviceSelection GetRenderSelection() const = 0;

    virtual int Init(int xScr,int yScr,int mode, void* wnd = nullptr, int RefreshRateInHz=0);
    virtual int Done();

    virtual bool ChangeSize(int xScr,int yScr,int mode) = 0;
    virtual int GetClipRect(int *xmin,int *ymin,int *xmax,int *ymax) = 0;
    virtual int SetClipRect(int xmin,int ymin,int xmax,int ymax) = 0;
    virtual void UseOrthographicProjection() = 0;
    virtual void SetDrawTransform(class cCamera *pDrawNode) = 0;
    virtual void SetWorldMat4f(const Mat4f* matrix) = 0;

    virtual int BeginScene() = 0;
    virtual int EndScene() = 0;
    virtual int Fill(int r,int g,int b,int a=255) = 0;
    virtual int Flush(bool wnd=false) = 0;
    virtual int SetGamma(float fGamma,float fStart=0.f,float fFinish=1.f) = 0;

    // вспомогательные функции, могут быть не реализованы
    virtual void DrawLine(int x1,int y1,int x2,int y2,sColor4c color) = 0;
    virtual void DrawPixel(int x1,int y1,sColor4c color) = 0;
    virtual void DrawRectangle(int x,int y,int dx,int dy,sColor4c color,bool outline=false) = 0;
    virtual void FlushPrimitive2D() = 0;
    virtual void DrawLine(const Vect3f &v1,const Vect3f &v2,sColor4c color) = 0;
    virtual void DrawPoint(const Vect3f &v1,sColor4c color) = 0;
    virtual void FlushPrimitive3D() = 0;

    /*Внутренний мтод. Использовать с крайней осторожностью.
      Перед использованием посмотреть как используется внутри Render.
    */
    virtual void ChangeTextColor(const char* &str,sColor4c& diffuse);

    virtual void OutTextRect(int x,int y,const char *string,int align,Vect2f& bmin,Vect2f& bmax);
    
    virtual void OutText(int x,int y,const char *string,const sColor4f& color,int align=-1,eBlendMode blend_mode=ALPHA_BLEND) = 0;
    virtual void OutText(int x,int y,const char *string,const sColor4f& color,int align,eBlendMode blend_mode,
                         cTexture* pTexture,eColorMode mode,
                         Vect2f uv,//Координаты текстуры в точке x,y
                         Vect2f duv,//du,dv на один логический пиксель 
            //(лог пиксель равен графическому в разрешении 1024x768)
                         float phase=0,
                         float lerp_factor=1//0..1 Насколько сильно влияет pTexture
    ) = 0;

    virtual bool SetScreenShot(const char *fname) = 0;
    virtual int GetRenderState(eRenderStateOption option) = 0;
    virtual int SetRenderState(eRenderStateOption option,int value) = 0;
    virtual void DrawBound(const MatXf &Matrix,Vect3f &min,Vect3f &max,bool wireframe=0,const sColor4c& Color=sColor4c(255,255,255,255)) = 0;

    virtual int Create(class cTileMap *TileMap) = 0;
    virtual void PreDraw(cTileMap *TileMap) = 0;
    virtual void Draw(cTileMap *TileMap,eBlendMode MatMode,TILEMAP_DRAW tile_draw,bool shadow) = 0;
    virtual int Delete(class cTileMap *TileMap) = 0;
    
    virtual void DrawSprite(int x,int y,int dx,int dy,float u,float v,float du,float dv,
                            cTexture *Texture,const sColor4c& ColorMul=sColor4c(255,255,255,255),float phase=0,eBlendMode mode=ALPHA_NONE) = 0;
    virtual void DrawSprite2(int x,int y,int dx,int dy,float u,float v,float du,float dv,
                             cTexture *Tex1,cTexture *Tex2,const sColor4c& ColorMul=sColor4c(255,255,255,255),float phase=0);
    virtual void DrawSprite2(int x,int y,int dx,int dy,float u,float v,float du,float dv,float u1,float v1,float du1,float dv1,
                             cTexture *Tex1,cTexture *Tex2,const sColor4c& ColorMul=sColor4c(255,255,255,255),float phase=0,eColorMode mode=COLOR_MOD,eBlendMode blend_mode=ALPHA_NONE) = 0;
    virtual void DrawSprite2(int x,int y,int dx,int dy,float u,float v,float du,float dv,float u1,float v1,float du1,float dv1,
                             cTexture *Tex1,cTexture *Tex2,float lerp_factor,float alpha=1,float phase=0,eColorMode mode=COLOR_MOD,eBlendMode blend_mode=ALPHA_NONE) = 0;
    
    virtual void Draw(class cScene *Scene) = 0;

    virtual void Draw(class FieldDispatcher *ffd) = 0;
    virtual void CreateFFDData(class FieldDispatcher *rd) = 0;
    virtual void DeleteFFDData(class FieldDispatcher *rd) = 0;
    virtual void Draw(class ElasticSphere *es) = 0;

    virtual int CreateTexture(class cTexture *Texture,class cFileImage *FileImage,bool enable_assert=true) = 0;
    virtual int DeleteTexture(class cTexture *Texture) = 0;
    virtual void* LockTexture(class cTexture *Texture, int& Pitch) = 0;
    virtual void UnlockTexture(class cTexture *Texture) = 0;
    
    virtual void SetGlobalFog(const sColor4f &color,const Vect2f &v) = 0;
    virtual void SetGlobalLight(Vect3f *vLight, sColor4f *Ambient = nullptr,
                                sColor4f *Diffuse = nullptr, sColor4f *Specular = nullptr) = 0;

    virtual bool IsEnableSelfShadow() = 0;
    
    virtual void SetNoMaterial(eBlendMode blend,float Phase=0,cTexture *Texture0=0,cTexture *Texture1=0,eColorMode color_mode=COLOR_MOD) = 0;
    virtual void DrawIndexedPrimitive(class VertexBuffer &vb, int OfsVertex, int nVertex, const class IndexBuffer& ib, int nOfsPolygon, int nPolygon) = 0;

    virtual void CreateVertexBuffer(class VertexBuffer &vb, uint32_t NumberVertex, vertex_fmt_t fmt, bool dynamic) = 0;
    virtual void DeleteVertexBuffer(class VertexBuffer &vb) = 0;
    virtual void* LockVertexBuffer(class VertexBuffer &vb) = 0;
    virtual void* LockVertexBuffer(class VertexBuffer &vb, uint32_t Start, uint32_t Amount) = 0;
    virtual void UnlockVertexBuffer(class VertexBuffer &vb) = 0;
    virtual void CreateIndexBuffer(class IndexBuffer& ib, uint32_t NumberIndices) = 0;
    virtual void DeleteIndexBuffer(class IndexBuffer &ib) = 0;
    virtual indices_t* LockIndexBuffer(class IndexBuffer &ib) = 0;
    virtual indices_t* LockIndexBuffer(class IndexBuffer &ib, uint32_t Start, uint32_t Amount) = 0;
    virtual void UnlockIndexBuffer(class IndexBuffer &ib) = 0;
    virtual void SubmitDrawBuffer(class DrawBuffer* db) = 0;
};

cInterfaceRenderDevice* CreateIRenderDevice(eRenderDeviceSelection selection);

extern cInterfaceRenderDevice* gb_RenderDevice;

//Из RenderDevice.h
enum eSurfaceFormat
{
    SURFMT_COLOR		=	0,
    SURFMT_COLORALPHA,
    SURFMT_RENDERMAP16,
    SURFMT_RENDERMAP32,
    SURFMT_BUMP,
    SURFMT_COLOR32,
    SURFMT_COLORALPHA32,
    SURFMT_RENDERMAP_FLOAT,
    SURFMT_GRAYALPHA,//A8L8 - в лудшем случае
    SURFMT_UV,
    SURFMT_U16V16,
    SURFMT_NUMBER
};

enum eMaterialMode
{
    MAT_COLOR_ADD_SPECULAR	=1<<4,

    MAT_ALPHA_ADDBLENDALPHA	=1<<5,
    MAT_ALPHA_BLEND			=1<<6,
    MAT_ALPHA_ADDBLEND		=1<<7,
    MAT_ALPHA_SUBBLEND		=1<<29,
    MAT_BUMP				=1<<9,
    MAT_NORMAL				=1<<10,

    MAT_ALPHA_TEST			=1<<8,

    MAT_IS_BLEND			= MAT_ALPHA_ADDBLENDALPHA|MAT_ALPHA_BLEND|MAT_ALPHA_ADDBLEND|MAT_ALPHA_SUBBLEND,
    // only d3d version
    // render type
    MAT_TEXMATRIX_STAGE1	=1<<16,
    MAT_TEXNORMAL_STAGE2	=1<<18,

    MAT_RENDER_SPHEREMAP	=1<<22,
    MAT_LIGHT				=1<<31
};

struct sDataRenderMaterial
{
    sColor4f	Ambient;
    sColor4f	Diffuse;
    sColor4f	Specular;
    sColor4f	Emissive;
    float		Power;

    float Phase;
    int			mat;//eMaterialMode
    cTexture	*Tex[2];
    MatXf		TexMatrix;
    float		MaterialAnimPhase;

    sDataRenderMaterial()			{ Phase=0; MaterialAnimPhase=0; }
};

bool GetAllTriangle(const char* filename, std::vector<Vect3f>& point, std::vector<sPolygon>& polygon);

/************************************************************************
* Copyright (c) 2005-2007 tok@openlinux.org.uk                          *
*                                                                       *
* This file contains code derived from information copyrighted by       *
* DMA Design. It may not be used in a commercial product.               *
*                                                                       *
* See license.txt for details.                                          *
*                                                                       *
* This notice may not be removed or altered.                            *
************************************************************************/
#ifndef OPENGTA_MAIN_H
#define OPENGTA_MAIN_H

#include <string>
#include <map>
#include <vector>
#include <SDL.h>
#include <physfs.h>
#include "set.h"

namespace OpenGTA {

  /** The common class for all graphics wrappers.
   * Contains a number of common variables; does essentially nothing.
   */
  class GraphicsBase {
    public:  
      GraphicsBase();
      virtual ~GraphicsBase();
      uint8_t getFormat();

      typedef struct ObjectInfo {
        PHYSFS_uint32 width, height, depth;
        PHYSFS_uint16 sprNum, weight, aux;
        PHYSFS_sint8 status;
        PHYSFS_uint8 numInto;
        //PHYSFS_uint16 into[255]; // FIXME: MAX_INTO ???
      } ObjectInfo;
      typedef struct LoadedAnim {
        LoadedAnim(size_t size) : frame(size) {}
        PHYSFS_uint8 block;
        PHYSFS_uint8 which;
        PHYSFS_uint8 speed;
        PHYSFS_uint8 frameCount;
        std::vector<PHYSFS_uint8> frame;
      } LoadedAnim;

      typedef struct DoorInfo {
        PHYSFS_sint16 rpx, rpy;
        PHYSFS_sint16 object;
        PHYSFS_sint16 delta;
      } DoorInfo;

      typedef struct HlsInfo {
        PHYSFS_sint16 h, l, s;
      } HlsInfo;

      typedef struct CarInfo {
        PHYSFS_sint16 width, height, depth;
        PHYSFS_sint16 sprNum;
        PHYSFS_sint16 weightDescriptor;
        PHYSFS_sint16 maxSpeed, minSpeed;
        PHYSFS_sint16 acceleration, braking;
        PHYSFS_sint16 grip, handling;
        // ... remaps
        HlsInfo    remap24[12];
        PHYSFS_uint8  remap8[12];
        PHYSFS_uint8 vtype;
        PHYSFS_uint8 model;
        PHYSFS_uint8 turning;
        PHYSFS_uint8 damagable;
        PHYSFS_uint16 value[4];
        PHYSFS_sint8 cx,cy;
        PHYSFS_uint32 moment;
        float rbpMass;
        float g1_Thrust;
        float tyreAdhesionX, tyreAdhesionY;
        float handBrakeFriction;
        float footBrakeFriction;
        float frontBrakeBias;
        PHYSFS_sint16 turnRatio;
        PHYSFS_sint16 driveWheelOffset;
        PHYSFS_sint16 steeringWheelOffset;
        float backEndSlideValue;
        float handBrakeSlideValue;
        PHYSFS_uint8 convertible;
        PHYSFS_uint8 engine;
        PHYSFS_uint8 radio;
        PHYSFS_uint8 horn;
        PHYSFS_uint8 soundFunction;
        PHYSFS_uint8 fastChangeFlag;
        PHYSFS_sint16 numDoors;
        DoorInfo door[4]; // FIXME: MAX_DOORS
      } CarInfo;
      /*
       * float->fixed:
       *  fixed = int(floatnum * 65536)
       *
       * fixed->float
       *  float = float(fixedNum)/65536
       *
       *  int->fixed
       *   fixed = intNum << 16
       *
       *  fixed->int
       *   int = fixedNum >> 16
       */

      typedef struct DeltaInfo {
        PHYSFS_uint16 size;
        unsigned char* ptr;
      } DeltaInfo;
      
      typedef struct SpriteInfo {
        PHYSFS_uint8 w;
        PHYSFS_uint8 h;
        PHYSFS_uint8 deltaCount;
        PHYSFS_uint16 size;
        PHYSFS_uint16 clut;
        PHYSFS_uint8  xoffset;
        PHYSFS_uint8  yoffset;
        PHYSFS_uint16 page;
        //unsigned char* ptr;
        DeltaInfo delta[33]; //FIXME: GTA_SPRITE_MAX_DELTAS
      } SpriteInfo;
      
      typedef struct SpriteNumbers {
        PHYSFS_uint16 GTA_SPRITE_ARROW;
        PHYSFS_uint16 GTA_SPRITE_DIGITS;
        PHYSFS_uint16 GTA_SPRITE_BOAT;
        PHYSFS_uint16 GTA_SPRITE_BOX;
        PHYSFS_uint16 GTA_SPRITE_BUS;
        PHYSFS_uint16 GTA_SPRITE_CAR;
        PHYSFS_uint16 GTA_SPRITE_OBJECT;
        PHYSFS_uint16 GTA_SPRITE_PED;
        PHYSFS_uint16 GTA_SPRITE_SPEEDO;
        PHYSFS_uint16 GTA_SPRITE_TANK;
        PHYSFS_uint16 GTA_SPRITE_TRAFFIC_LIGHTS;
        PHYSFS_uint16 GTA_SPRITE_TRAIN;
        PHYSFS_uint16 GTA_SPRITE_TRDOORS;
        PHYSFS_uint16 GTA_SPRITE_BIKE;
        PHYSFS_uint16 GTA_SPRITE_TRAM;
        PHYSFS_uint16 GTA_SPRITE_WBUS;
        PHYSFS_uint16 GTA_SPRITE_WCAR;
        PHYSFS_uint16 GTA_SPRITE_EX;
        PHYSFS_uint16 GTA_SPRITE_TUMCAR;
        PHYSFS_uint16 GTA_SPRITE_TUMTRUCK;
        PHYSFS_uint16 GTA_SPRITE_FERRY;

        enum SpriteTypes {
          ARROW = 0,
          DIGIT,
          BOAT,
          BOX,
          BUS,
          CAR,
          OBJECT,
          PED,
          SPEEDO,
          TANK,
          TRAFFIC_LIGHT,
          TRAIN,
          TRDOOR,
          BIKE,
          TRAM,
          WBUS,
          WCAR,
          EX,
          TUMCAR,
          TUMTRUCK,
          FERRY
        };

        PHYSFS_uint16 reIndex(const PHYSFS_uint16 & id, const enum SpriteTypes & st) const;
        PHYSFS_uint16 countByType(const SpriteTypes & t) const;
      } SpriteNumbers;

      bool isAnimatedBlock(uint8_t area_code, uint8_t id);

      void prepareSideTexture(unsigned int idx, unsigned char* dst);
      void prepareLidTexture(unsigned int idx, unsigned char* dst);
      void prepareAuxTexture(unsigned int idx, unsigned char* dst);
      unsigned int getRandomPedRemapNumber();
      unsigned int getPedRemapNumberType(unsigned int _type);
      
      SpriteNumbers spriteNumbers;

      CarInfo* findCarByModel(PHYSFS_uint8);
      size_t   getNumCarModels() { return carInfos.size(); }
      unsigned char* getTmpBuffer(bool rgba);
      SpriteInfo* getSprite(size_t id) { return spriteInfos[id]; }

      virtual unsigned char* getSide(unsigned int idx, unsigned int palIdx, bool rgba) = 0;
      virtual unsigned char* getLid(unsigned int idx, unsigned int palIdx, bool rgba) = 0;
      virtual unsigned char* getAux(unsigned int idx, unsigned int palIdx, bool rgba) = 0;

      virtual unsigned char* getSpriteBitmap(size_t id, int remap, Uint32 delta) = 0;
      
      std::vector<LoadedAnim*> animations;
      std::vector<SpriteInfo*> spriteInfos;
      std::vector<ObjectInfo*> objectInfos;
      std::vector<CarInfo*>    carInfos;

      bool getDeltaHandling();
      void setDeltaHandling(bool delta_as_set);

      bool isBlockingSide(uint8_t id);
      void setupBlocking(const std::string & file);

    protected:
      void loadTileTextures();
      void loadAnim();

      void loadObjectInfo_shared(PHYSFS_uint64 offset);
      void loadSpriteNumbers_shared(PHYSFS_uint64 offset);
      void loadCarInfo_shared(PHYSFS_uint64 offset);
      void loadSpriteInfo_shared(PHYSFS_uint64 offset);

      void handleDeltas(const SpriteInfo & spriteinfo, unsigned char* buffer,
        Uint32 delta);
      void applyDelta(const SpriteInfo & spriteInfo, unsigned char* buffer, 
        Uint32 offset, const DeltaInfo & deltaInfo, bool mirror = false);

      PHYSFS_file* fd;
      unsigned char* rawTiles;
      unsigned char* rawSprites;

      PHYSFS_uint32 sideSize;
      PHYSFS_uint32 lidSize;
      PHYSFS_uint32 auxSize;
      PHYSFS_uint32 animSize;
      PHYSFS_uint32 objectInfoSize;
      PHYSFS_uint32 carInfoSize;
      PHYSFS_uint32 spriteInfoSize;
      PHYSFS_uint32 spriteGraphicsSize;
      PHYSFS_uint32 spriteNumberSize;
      
      PHYSFS_uint32 auxBlockTrailSize;

      /*
      int loadSide();
      int loadLid();
      int loadAux();
      int loadAnim();
      int loadObject();
      int loadCar();
      int loadSpriteInfo();
      int loadSpriteGraphics();
      int loadSpriteNumbers();*/

      PHYSFS_uint8 _topHeaderSize;
      
      unsigned char tileTmp[4096];
      unsigned char tileTmpRGB[4096*3];
      unsigned char tileTmpRGBA[4096*4];

      bool delta_is_a_set;

      Util::Set sideTexBlockMove;

      unsigned int firstValidPedRemap;
      unsigned int lastValidPedRemap;
  };

  // just a forward declaration
  class CityView;
  
  /** Loader for STYLE*.GRY files.
   * 
   * Implements loading the 8-bit graphic files.
   */
  class Graphics8Bit : public GraphicsBase {
    /** allow renderer direct access to members */
    friend class CityView;
    public:
      /** Constructor for graphics loader.
       * @param style a valid filename (maybe uppercase depending on your files)
       */
      Graphics8Bit(const std::string& style);
      /** Destructor cleans all rawdata caches. */
      ~Graphics8Bit();
      
      /** Helper to apply palettes to various raw bitmaps.
       * @see Graphics8Bit
       * @see Font
       */
      class RGBPalette {
        private:
          unsigned char data[256*3];
        public:
          /** Empty constructor.
           * You HAVE to call loadFromFile() function when using this constructor!.
           */
          RGBPalette();
          /** Formerly private member, now exposed for Font class; take care.
           * @param fd PHYSFS_file* handle.
           */
          int loadFromFile(PHYSFS_file* fd);
          /** Constructor from PHYFS_file.
           * @param fd PHYSFS_file* handle
           */
          RGBPalette(PHYSFS_file* fd);
          /** Constructor from filename.
           * @param filename a palette file name
           */
          RGBPalette(const std::string& palette);
          /** Transforms an input buffer using the palette stored in this instance.
           * @param len length of the src buffer (in byte)
           * @param src pointer to src buffer
           * @param dst pointer to dst buffer (must exist and be large enough)
           * @param rgba use 'true' to create a RGBA image, or 'false' (default) for RGB
           */
          void apply(unsigned int len, const unsigned char* src, unsigned char* dst, bool rgba = false);
      };
      
      unsigned char* getSide(unsigned int idx, unsigned int palIdx, bool rgba);
      unsigned char* getLid(unsigned int idx, unsigned int palIdx, bool rgba);
      unsigned char* getAux(unsigned int idx, unsigned int palIdx, bool rgba);

      unsigned char* getSpriteBitmap(size_t id, int remap, Uint32 delta);

      void dump();

    private:
      PHYSFS_uint32 paletteSize;
      PHYSFS_uint32 remapSize;
      PHYSFS_uint32 remapIndexSize;
    protected:
      void loadHeader();
      void loadPalette();
      void loadRemapTables();
      void loadRemapIndex();
      void loadObjectInfo();
      void loadCarInfo();
      void loadSpriteInfo();
      void loadSpriteGraphics();
      void loadSpriteNumbers();
      void applyRemap(unsigned int len, unsigned int which, unsigned char* buffer);
      RGBPalette* masterRGB;
      PHYSFS_uint8 remapTables[256][256];
      PHYSFS_uint8 remapIndex[256][4];
      
  };

  class Graphics24Bit : public GraphicsBase {
    public:
      Graphics24Bit(const std::string & style);
      ~Graphics24Bit();

      unsigned char* getSide(unsigned int idx, unsigned int palIdx, bool rgba);
      unsigned char* getLid(unsigned int idx, unsigned int palIdx, bool rgba);
      unsigned char* getAux(unsigned int idx, unsigned int palIdx, bool rgba);

      unsigned char* getSpriteBitmap(size_t id, int remap, Uint32 delta);

      void dumpClut(const char* fname);

    protected:
      void loadHeader();
      void loadClut();
      void loadPalIndex();
      void loadObjectInfo();
      void loadCarInfo();
      void loadSpriteInfo();
      void loadSpriteGraphics();
      void loadSpriteNumbers();

      void applyClut(unsigned char* src, unsigned char* dst, 
        const size_t & len, const PHYSFS_uint16 & clutIdx, bool rgba);

    private:
      PHYSFS_uint32 clutSize;
      PHYSFS_uint32 pagedClutSize; 
      PHYSFS_uint32 tileclutSize;
      PHYSFS_uint32 spriteclutSize;
      PHYSFS_uint32 newcarclutSize;
      PHYSFS_uint32 fontclutSize;
      PHYSFS_uint32 paletteIndexSize;

      unsigned char* rawClut;
      PHYSFS_uint16* palIndex;
  };




}
#endif

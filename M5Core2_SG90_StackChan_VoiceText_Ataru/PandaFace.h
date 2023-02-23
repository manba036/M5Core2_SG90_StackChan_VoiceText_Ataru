#ifndef PANDAFACE_H_
#define PANDAFACE_H_

#include <Face.h>
#include <Avatar.h>
#include <M5GFX.h>
#include "BoundingRect.h"
#include "DrawContext.h"
#include "Drawable.h"

using namespace m5avatar;

class PandaEye : public Drawable {
  void draw(M5Canvas *spi, BoundingRect rect, DrawContext *ctx) {
    Expression exp = ctx->getExpression();
    uint32_t cx = rect.getCenterX();
    uint32_t cy = rect.getCenterY();
    Gaze g = ctx->getGaze();
    ColorPalette *cp = ctx->getColorPalette();
    uint16_t primaryColor = cp->get(COLOR_PRIMARY);
    uint16_t backgroundColor = ctx->getColorDepth() == 1 ? ERACER_COLOR : cp->get(COLOR_BACKGROUND);
    uint32_t offsetX = g.getHorizontal() * 20;
    uint32_t offsetY = g.getVertical() * 5;
    float eor = ctx->getEyeOpenRatio();

    int16_t w = rect.getWidth();
    int16_t h = rect.getHeight();
    int16_t d = 50;//min(w, h);
    int16_t di = d / 3;
    spi->fillEllipse(cx, cy, d, d, primaryColor);
    if (eor == 0) {
      // eye closed
      for (int i = 0; i < 8; i++) {
        spi->fillEllipse(cx, cy + i, di, di, TFT_BLACK);
        spi->fillEllipse(cx, cy + i + 1, di, di, primaryColor);
      }
    } else {
      spi->fillEllipse(cx + offsetX, cy + offsetY, di, di, backgroundColor);
    }
  }
};

class PandaMouth : public Drawable {
 private:
  uint16_t minWidth;
  uint16_t maxWidth;
  uint16_t minHeight;
  uint16_t maxHeight;

 public:
  PandaMouth() : PandaMouth(50, 90, 4, 60) {}
  PandaMouth(uint16_t minWidth, uint16_t maxWidth, uint16_t minHeight,
           uint16_t maxHeight)
      : minWidth{minWidth},
        maxWidth{maxWidth},
        minHeight{minHeight},
        maxHeight{maxHeight} {}
  void draw(M5Canvas *spi, BoundingRect rect, DrawContext *ctx) {
    uint16_t primaryColor = ctx->getColorPalette()->get(COLOR_PRIMARY);
    uint16_t backgroundColor = ctx->getColorDepth() == 1 ? ERACER_COLOR : ctx->getColorPalette()->get(COLOR_BACKGROUND);
    uint32_t cx = rect.getCenterX();
    uint32_t cy = rect.getCenterY();
    float openRatio = ctx->getMouthOpenRatio();
    uint32_t h = minHeight + (maxHeight - minHeight) * openRatio;
    uint32_t w = minWidth + (maxWidth - minWidth) * (1 - openRatio);
    if (h > minHeight) {
      spi->fillEllipse(cx, cy, w / 2, h / 2, primaryColor);
      spi->fillEllipse(cx, cy, w / 2 - 4, h / 2 - 4, TFT_RED);
      spi->fillRect(cx - w / 2, cy - h / 2, w, h / 2, backgroundColor);
    }
    spi->fillEllipse(cx, cy - 15, 10, 6, primaryColor);
    spi->fillEllipse(cx - 28, cy, 30, 15, primaryColor);
    spi->fillEllipse(cx + 28, cy, 30, 15, primaryColor);
    spi->fillEllipse(cx - 29, cy - 4, 27, 15, backgroundColor);
    spi->fillEllipse(cx + 29, cy - 4, 27, 15, backgroundColor);
  }
};

class PandaFace : public Face {
 public:
  PandaFace()
      : Face(new PandaMouth(), new BoundingRect(168, 163), new PandaEye(),
             new BoundingRect(103, 80), new PandaEye(),
             new BoundingRect(106, 240), new Eyeblow(15, 2, false),
             new BoundingRect(67, 96), new Eyeblow(15, 2, true),
             new BoundingRect(72, 230)) {}
};

#endif  // PANDAFACE_H_
#!/usr/bin/env python3

from dataclasses import dataclass
from pathlib import Path

import cv2 as cv
import numpy as np

# based on https://www.thepythoncode.com/article/kmeans-for-image-segmentation-opencv-python

TILE_WIDTH = 8
K = 64

REPO = Path(__file__).parent.parent
RES_RAW = REPO / 'res_raw'
SRC = RES_RAW / 'nasa_sun_shrunk.png'
#SRC = "/Users/zacharyrubenstein/Desktop/original_image_resized.png"

@dataclass
class Tile:
    tile_row: int
    tile_col: int
    pixel_row: int
    pixel_col: int
    img: cv.Mat
    flat: np.array

img = cv.imread(str(SRC))
h, w, _ = img.shape
assert h % TILE_WIDTH == 0 and w % TILE_WIDTH == 0, f'height and width must be multiples of {TILE_WIDTH}'

tiles = []
for r in range(h // TILE_WIDTH):
    for c in range(w // TILE_WIDTH):
        pixel_row = r * TILE_WIDTH
        pixel_col = c * TILE_WIDTH
        tile_img = img[pixel_row:pixel_row + TILE_WIDTH, pixel_col:pixel_col + TILE_WIDTH]
        tiles.append(Tile(
            tile_row=r,
            tile_col=c,
            pixel_row=pixel_row,
            pixel_col=pixel_col,
            img=tile_img,
            flat=cv.cvtColor(
                tile_img,
                cv.COLOR_BGR2HSV,
                ).reshape(-1).astype(np.float32),
            ))
input_array = np.array([t.flat for t in tiles])
criteria = (cv.TERM_CRITERIA_EPS + cv.TERM_CRITERIA_MAX_ITER, 10000, 0.001)
_, labels, (centers) = cv.kmeans(input_array, K, None, criteria, 100, cv.KMEANS_RANDOM_CENTERS)
centers_reshaped = [cv.cvtColor(c.astype(np.uint8).reshape(TILE_WIDTH, TILE_WIDTH, 3), cv.COLOR_HSV2BGR) for c in centers]
new_img = np.zeros(img.shape)
for l, t in zip(labels, tiles):
    new_img[t.pixel_row:t.pixel_row + TILE_WIDTH, t.pixel_col:t.pixel_col + TILE_WIDTH] = centers_reshaped[l[0]]
        
cv.imshow("Display window", new_img)
k = cv.waitKey(0)

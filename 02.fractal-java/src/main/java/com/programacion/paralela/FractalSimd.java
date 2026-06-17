package com.programacion.paralela;

import java.nio.ByteBuffer;

public class FractalSimd {

    ByteBuffer pixelBuffer;

    public FractalSimd() {
        this.pixelBuffer = ByteBuffer.allocateDirect(FractalParams.WIDTH * FractalParams.HEIGHT * 4); // 4 bytes per pixel (RGBA)
    }

    public void juliaSimd() {
        FractalDll.INSTANCE.julia_simd(
                FractalParams.xMin, FractalParams.yMin,
                FractalParams.xMax, FractalParams.yMax,
                FractalParams.WIDTH, FractalParams.HEIGHT,
                FractalParams.max_iteraciones, pixelBuffer);
    };

}
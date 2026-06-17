package com.programacion.paralela;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class FractalParallel {
    public int[] pixel_buffer;
    private final int numThreads;

    public FractalParallel() {
        this.pixel_buffer = new int[FractalParams.WIDTH * FractalParams.HEIGHT];
        this.numThreads = Runtime.getRuntime().availableProcessors();
    }

    public int getNumThreads() {
        return numThreads;
    }

    private int acotado_2(double x, double y) {
        int iter = 1;
        double zr = x;
        double zi = y;

        while (iter < FractalParams.max_iteraciones && (zr * zr + zi * zi) < 4.0) {
            double dr = zr * zr - zi * zi + FractalParams.cReal;
            double di = 2.0 * zr * zi + FractalParams.cImag;
            zr = dr;
            zi = di;
            iter++;
        }

        if (iter < FractalParams.max_iteraciones) {
            int index = iter % FractalParams.PALETTE_SIZE;
            return FractalParams.color_ramp_parallel[index];
        }

        return 0xFF000000; // Negro
    }

    public void julia_parallel(double x_min, double y_min, double x_max, double y_max, int width, int height) {
        ExecutorService executor = Executors.newFixedThreadPool(numThreads);
        
        int rowsPerThread = height / numThreads;

        for (int t = 0; t < numThreads; t++) {
            final int startRow = t * rowsPerThread;
            final int endRow = (t == numThreads - 1) ? height : (t + 1) * rowsPerThread;

            executor.execute(() -> {
                double dx = (x_max - x_min) / width;
                double dy = (y_max - y_min) / height;

                for (int j = startRow; j < endRow; j++) {
                    for (int i = 0; i < width; i++) {
                        double x = x_min + i * dx;
                        double y = y_max - j * dy;
                        pixel_buffer[j * width + i] = acotado_2(x, y);
                    }
                }
            });
        }

        executor.shutdown();
        try {
            executor.awaitTermination(1, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}

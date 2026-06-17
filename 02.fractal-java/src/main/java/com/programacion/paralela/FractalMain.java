package com.programacion.paralela;


import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.util.Objects;

import org.lwjgl.*;
import org.lwjgl.glfw.*;
import org.lwjgl.opengl.*;
import org.lwjgl.system.*;

import static org.lwjgl.glfw.Callbacks.*;
import static org.lwjgl.glfw.GLFW.*;
import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.opengl.GL12.*;
import static org.lwjgl.system.MemoryUtil.*;

public class FractalMain {

    //windos handle
    private int textureID;
    private int overlayTextureID;
    private long window;

    private final IntBuffer pixelBuffer;

    private GLFWKeyCallback keyCallback;


    FractalCpu fractalCpu;
    FractalSimd fractalSimd;
    FractalParallel fractalParallel;

    FPSCounter fpsCounter;

    int modo = 1; //1: CPU, 2: simd, 3: parallel


    public FractalMain() {
        fractalCpu = new FractalCpu();
        fractalSimd = new FractalSimd();
        fractalParallel = new FractalParallel();
        fpsCounter = new FPSCounter();

        pixelBuffer = BufferUtils.createIntBuffer(FractalParams.WIDTH * FractalParams.HEIGHT);
    }


    public void run() {
        System.out.println("Fractal Julia " + Version.getVersion());

        init();
        loop();

        glfwFreeCallbacks(window);
        glfwDestroyWindow(window);

        glfwTerminate();
        Objects.requireNonNull(glfwSetErrorCallback(null)).free();
        //glfwSetErrorCallback(null).free();
    }

    private void init() {

        GLFWErrorCallback errorCallback = GLFWErrorCallback.createPrint(System.err).set();

        if ( !glfwInit() )
            throw new IllegalStateException("Unable to initialize GLFW");

        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        // Create the window
        window = glfwCreateWindow(FractalParams.WIDTH, FractalParams.HEIGHT, "Julia Set", 0, 0);
        if ( window == NULL )
            throw new RuntimeException("Failed to create the GLFW window");

        //manejo de eventos
        glfwSetKeyCallback(window, (window, key, scancode, action, mods) -> {
            if ( key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE )
                glfwSetWindowShouldClose(window, true);
            if (key==GLFW_KEY_UP && action == GLFW_RELEASE ){
                FractalParams.max_iteraciones +=10;
            }
            if (key==GLFW_KEY_DOWN && action == GLFW_RELEASE ){
                FractalParams.max_iteraciones -=10;
                if (FractalParams.max_iteraciones <0) FractalParams.max_iteraciones = 10;
            }
            if(key==GLFW_KEY_1 && action==GLFW_RELEASE){
                System.out.println("Modo Java CPU");
                modo=1;
            }
            else if (key==GLFW_KEY_2 && action==GLFW_RELEASE) {
                System.out.println("Modo C/C++ SIMD");
                modo=2;
                fractalSimd = new FractalSimd();
            }
            else if (key==GLFW_KEY_3 && action==GLFW_RELEASE) {
                System.out.println("Modo Java Multi-thread (" + fractalParallel.getNumThreads() + " cores)");
                modo=3;
            }


        });
        GLFWVidMode vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        assert vidmode != null;
        glfwSetWindowPos(window,
                (vidmode.width()-FractalParams.WIDTH)/2,
                (vidmode.height()-FractalParams.HEIGHT)/2
        );

        // Make the OpenGL context current
        glfwMakeContextCurrent(window);

        GL.createCapabilities();
        GL.createCapabilitiesWGL(); //usado para windows

        //----version de OpenGL
        String version = GL11.glGetString(GL11.GL_VERSION);
        String vendor = GL11.glGetString(GL11.GL_VENDOR);
        String renderer = GL11.glGetString(GL11.GL_RENDERER);

        System.out.println("OpenGL version: " + version);
        System.out.println("OpenGL vendor: " + vendor);
        System.out.println("OpenGL renderer: " + renderer);




        //--conf
        GL11.glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1,1,-1,1,-1,1);

        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_TEXTURE_2D);
        glLoadIdentity();

        // Enable v-sync
        glfwSwapInterval(1);

        // Make the window visible
        glfwShowWindow(window);

        setupTexture();
        setupOverlayTexture();

    }

    private void setupTexture() {
        textureID = glGenTextures();
        glBindTexture(GL_TEXTURE_2D, textureID);

        //RESERVA LA MEMORIA UNICAMENTE
        glTexImage2D(
                GL_TEXTURE_2D,0,
                GL_RGBA8,
                FractalParams.WIDTH,
                FractalParams.HEIGHT,
                0,
                GL_BGRA,
                GL_UNSIGNED_BYTE,
                NULL
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    }

    private void setupOverlayTexture() {
        overlayTextureID = glGenTextures();
        glBindTexture(GL_TEXTURE_2D, overlayTextureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    private void loop() {

        GL.createCapabilities();

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        while ( !glfwWindowShouldClose(window) ) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            paint();

            glfwSwapBuffers(window);

            glfwPollEvents();
        }
    }

    private  void paint() {

        int fps = fpsCounter.update();
        System.out.println("FPS: " + fps);

        pixelBuffer.clear();

        if(modo==1) {
            fractalCpu.julia_serial_2(FractalParams.xMin,FractalParams.yMin,FractalParams.xMax,FractalParams.yMax,FractalParams.WIDTH,FractalParams.HEIGHT);
            pixelBuffer.put(fractalCpu.pixel_buffer);
        }
        else if (modo==2){
            fractalSimd.juliaSimd();
            pixelBuffer.put(fractalSimd.pixelBuffer.asIntBuffer());
        }
        else if (modo==3){
            fractalParallel.julia_parallel(FractalParams.xMin,FractalParams.yMin,FractalParams.xMax,FractalParams.yMax,FractalParams.WIDTH,FractalParams.HEIGHT);
            pixelBuffer.put(fractalParallel.pixel_buffer);
        }

        pixelBuffer.flip();

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D( GL_TEXTURE_2D,0,
                GL_RGBA8,
                FractalParams.WIDTH,
                FractalParams.HEIGHT,
                0,
                GL_BGRA,
                GL_UNSIGNED_BYTE,
                pixelBuffer);


        glBegin(GL_QUADS);
        {
            glTexCoord2d(0.0f, 0.0f);
            glVertex2d(-1,-1);


            glTexCoord2d(0.0f, 1f);
            glVertex2d(-1,1);

            glTexCoord2d(1f, 1f);
            glVertex2d(1,1);

            glTexCoord2d(1f, 0f);
            glVertex2d(1,-1);
        }
        glEnd();

        drawOverlay(fps);
    }

    private void drawOverlay(int fps) {
        BufferedImage image = new BufferedImage(FractalParams.WIDTH, FractalParams.HEIGHT, BufferedImage.TYPE_INT_ARGB);
        Graphics2D g = image.createGraphics();
        try {
            g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
            g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
            g.setColor(Color.WHITE);
            g.setFont(new Font("SansSerif", Font.BOLD, 24));
            String modeName = switch (modo) {
                case 1 -> "Serial Java";
                case 2 -> "SIMD C++";
                case 3 -> "Parallel (" + fractalParallel.getNumThreads() + " cores)";
                default -> "Desconocido";
            };
            g.drawString("Julia Set | Iteraciones: " + FractalParams.max_iteraciones + " | FPS: " + fps + " | Mode: " + modeName, 10, 28);

            g.setFont(new Font("SansSerif", Font.BOLD, 20));
            g.drawString("Options: [1-3] Mode | [Up/Down] Iterations | [Esc] Exit", 10, FractalParams.HEIGHT - 20);
        } finally {
            g.dispose();
        }

        ByteBuffer buffer = ByteBuffer.allocateDirect(FractalParams.WIDTH * FractalParams.HEIGHT * 4);
        int[] pixels = image.getRGB(0, 0, FractalParams.WIDTH, FractalParams.HEIGHT, null, 0, FractalParams.WIDTH);
        for (int argb : pixels) {
            buffer.put((byte) ((argb >> 16) & 0xFF));
            buffer.put((byte) ((argb >> 8) & 0xFF));
            buffer.put((byte) (argb & 0xFF));
            buffer.put((byte) ((argb >> 24) & 0xFF));
        }
        buffer.flip();

        glBindTexture(GL_TEXTURE_2D, overlayTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, FractalParams.WIDTH, FractalParams.HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindTexture(GL_TEXTURE_2D, overlayTextureID);
        glBegin(GL_QUADS);
        {
            glTexCoord2d(0.0f, 1f);
            glVertex2d(-1,-1);

            glTexCoord2d(0.0f, 0.0f);
            glVertex2d(-1,1);

            glTexCoord2d(1f, 0f);
            glVertex2d(1,1);

            glTexCoord2d(1f, 1f);
            glVertex2d(1,-1);
        }
        glEnd();
        glDisable(GL_BLEND);
    }


    //native significa que el metodo se implementa en codigo nativo (C/C++) y se llama desde Java
    public native void julia_simd(double val, double ymin);
    public static void main(String[] args) {
        new FractalMain().run();
    }

}
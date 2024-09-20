#include "openglspriterenderer.h"

#include "FlexEngine/AssetManager/assetmanager.h" // FLX_ASSET_GET
#include "FlexEngine/DataStructures/freequeue.h"

namespace FlexEngine
{

  // static member initialization
  uint32_t OpenGLSpriteRenderer::m_draw_calls = 0;
  uint32_t OpenGLSpriteRenderer::m_draw_calls_last_frame = 0;
  bool OpenGLSpriteRenderer::m_depth_test = false;
  bool OpenGLSpriteRenderer::m_blending = false;

  GLuint OpenGLSpriteRenderer::samples = 8;
  float OpenGLSpriteRenderer::gamma = 2.2f;
  GLuint OpenGLSpriteRenderer::m_rectVAO=0;
  GLuint OpenGLSpriteRenderer::m_rectVBO=0;
  GLuint OpenGLSpriteRenderer::m_postProcessingFBO = 0;
  GLuint OpenGLSpriteRenderer::m_colorBuffer = 0;
  GLuint OpenGLSpriteRenderer::bloomTexture = 0;
  GLuint OpenGLSpriteRenderer::postProcessingTexture = 0;
  GLuint OpenGLSpriteRenderer::m_brightBuffer = 0;
  GLuint OpenGLSpriteRenderer::m_pingpongFBO[2] = {};
  GLuint OpenGLSpriteRenderer::m_pingpongBuffer[2] = {};

  uint32_t OpenGLSpriteRenderer::GetDrawCalls()
  {
    return m_draw_calls;
  }

  uint32_t OpenGLSpriteRenderer::GetDrawCallsLastFrame()
  {
    return m_draw_calls_last_frame;
  }

  bool OpenGLSpriteRenderer::IsDepthTestEnabled()
  {
    return m_depth_test;
  }

  void OpenGLSpriteRenderer::EnablePostProcessing()
  {
      glBindFramebuffer(GL_FRAMEBUFFER, m_postProcessingFBO);
  }

  void OpenGLSpriteRenderer::DisablePostProcessing()
  {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void OpenGLSpriteRenderer::EnableDepthTest()
  {
    m_depth_test = true;
    glEnable(GL_DEPTH_TEST);
  }

  void OpenGLSpriteRenderer::DisableDepthTest()
  {
    m_depth_test = false;
    glDisable(GL_DEPTH_TEST);
  }

  bool OpenGLSpriteRenderer::IsBlendingEnabled()
  {
    return m_blending;
  }

  void OpenGLSpriteRenderer::EnableBlending()
  {
    m_blending = true;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  void OpenGLSpriteRenderer::DisableBlending()
  {
    m_blending = false;
    glDisable(GL_BLEND);
  }

  void OpenGLSpriteRenderer::ClearFrameBuffer()
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void OpenGLSpriteRenderer::ClearColor(const Vector4& color)
  {
    glClearColor(color.x, color.y, color.z, color.w);
    m_draw_calls_last_frame = m_draw_calls;
    m_draw_calls = 0;
  }

  void OpenGLSpriteRenderer::InitBloomFBO(const Vector2& windowSize)
  {
      //// Create FBO for bloom effect
      //glGenFramebuffers(1, &m_fbo);
      //glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

      //// Color buffer for normal rendering
      //glGenTextures(1, &m_colorBuffer);
      //glBindTexture(GL_TEXTURE_2D, m_colorBuffer);
      //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
      //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorBuffer, 0);

      //// Brightness buffer for bloom
      //glGenTextures(1, &m_brightBuffer);
      //glBindTexture(GL_TEXTURE_2D, m_brightBuffer);
      //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
      //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_brightBuffer, 0);

      //// Tell OpenGL to render to both color buffers
      //GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
      //glDrawBuffers(2, attachments);

      //// Ping-pong FBOs for Gaussian blur
      //glGenFramebuffers(2, m_pingpongFBO);
      //glGenTextures(2, m_pingpongBuffer);
      //for (GLuint i = 0; i < 2; i++)
      //{
      //    glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO[i]);
      //    glBindTexture(GL_TEXTURE_2D, m_pingpongBuffer[i]);
      //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
      //    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pingpongBuffer[i], 0);
      //}
      auto& asset_shader = FLX_ASSET_GET(Asset::Shader, R"(/shaders/finalComposite)");
      asset_shader.Use();
      glUniform1i(glGetUniformLocation(asset_shader.Get(), "screenTexture"), 0);
      glUniform1i(glGetUniformLocation(asset_shader.Get(), "bloomTexture"), 1);
      glUniform1f(glGetUniformLocation(asset_shader.Get(), "gamma"), gamma);
      asset_shader = FLX_ASSET_GET(Asset::Shader, R"(/shaders/GaussianBlur)");
      asset_shader.Use();
      glUniform1i(glGetUniformLocation(asset_shader.Get(), "screenTexture"), 0);

      // unit square
       // Flipped UVs for OpenGL
      //const float vertices[] = {
      //    //  Coords   // texCoords
      //     1.0f, -1.0f,  1.0f, 0.0f,
      //    -1.0f, -1.0f,  0.0f, 0.0f,
      //    -1.0f,  1.0f,  0.0f, 1.0f,

      //     1.0f,  1.0f,  1.0f, 1.0f,
      //     1.0f, -1.0f,  1.0f, 0.0f,
      //    -1.0f,  1.0f,  0.0f, 1.0f
      //}; //Move this outside

      const float vertices[] = {
          // Position        // TexCoords
          -1.0f, -1.f, 0.0f,   0.0f, 0.0f, // Bottom-left
           1.0f, -1.f, 0.0f,   1.0f, 0.0f, // Bottom-right
           1.0f,  1.f, 0.0f,   1.0f, 1.0f, // Top-right
           1.0f,  1.f, 0.0f,   1.0f, 1.0f, // Top-right
          -1.0f,  1.f, 0.0f,   0.0f, 1.0f, // Top-left
          -1.0f, -1.f, 0.0f,   0.0f, 0.0f  // Bottom-left
      };
      //Change to 1,
      //Disable for now
      {

          //// Enables the Depth Buffer
          //glEnable(GL_DEPTH_TEST);

          //// Enables Multisampling
          //glEnable(GL_MULTISAMPLE);

          //// Enables Cull Facing
          //glEnable(GL_CULL_FACE);
          //// Keeps front faces
          //glCullFace(GL_FRONT);
          //// Uses counter clock-wise standard
          //glFrontFace(GL_CCW);
      }

      // Set up VAO and VBO
      glGenVertexArrays(1, &m_rectVAO);
      glGenBuffers(1, &m_rectVBO);
      glBindVertexArray(m_rectVAO);
      glBindBuffer(GL_ARRAY_BUFFER, m_rectVBO);
      // Correct the usage of vertices in glBufferData
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
      // Position attribute (3 floats: x, y, z)
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); // 5 * sizeof(float) is correct for the stride.
      // Texture coordinates attribute (2 floats: u, v), starting after the position
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); // Offset to the 4th element (3 floats).

      // Create Frame Buffer Object
      glGenFramebuffers(1, &m_postProcessingFBO);
      glBindFramebuffer(GL_FRAMEBUFFER, m_postProcessingFBO);

      // Create Framebuffer Texture
      int width, height;
      //TODO HELP WHY (not supposed to multiply one)
      width = windowSize.x;
      height = windowSize.y;
      glGenTextures(1, &postProcessingTexture);
      glBindTexture(GL_TEXTURE_2D, postProcessingTexture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postProcessingTexture, 0);

      // Create Second Framebuffer Texture
      glGenTextures(1, &bloomTexture);
      glBindTexture(GL_TEXTURE_2D, bloomTexture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bloomTexture, 0);

      // Tell OpenGL we need to draw to both attachments
      unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
      glDrawBuffers(2, attachments);

      // Error checking framebuffer
      auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
      if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
          Log::Fatal("Post-Processing Framebuffer error: " + fboStatus);

      // Create Ping Pong Framebuffers for repetitive blurring
      glGenFramebuffers(2, m_pingpongFBO);
      glGenTextures(2, m_pingpongBuffer);
      for (unsigned int i = 0; i < 2; i++)
      {
          glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO[i]);
          glBindTexture(GL_TEXTURE_2D, m_pingpongBuffer[i]);
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
          glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pingpongBuffer[i], 0);

          fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
          if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
              Log::Fatal("Ping-Pong Framebuffer error: " + fboStatus);
      }

      //Unbind frame buffer
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void OpenGLSpriteRenderer::DrawTexture2D(const Renderer2DProps& props)
  {
    // unit square
    // Flipped UVs for OpenGL
    static const float vertices[] = {
      // Position        // TexCoords
      -0.5f, -0.5f, 0.0f,   1.0f, 0.0f, // Bottom-left
       0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // Bottom-right
       0.5f,  0.5f, 0.0f,   0.0f, 1.0f, // Top-right
       0.5f,  0.5f, 0.0f,   0.0f, 1.0f, // Top-right
      -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // Top-left
      -0.5f, -0.5f, 0.0f,   1.0f, 0.0f  // Bottom-left
    };

    static GLuint vao = 0, vbo = 0;
    if (vao == 0)
    {
      glGenVertexArrays(1, &vao);
      glGenBuffers(1, &vbo);

      glBindVertexArray(vao);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

      glBindVertexArray(0);

      // free in freequeue
      FreeQueue::Push(
        [=]()
        {
          glDeleteBuffers(1, &vbo);
          glDeleteVertexArrays(1, &vao);
        }
      );
    }

    // guard
    if (vao == 0 || props.shader == "" || props.scale == Vector2::Zero) return;

    // bind all
    glBindVertexArray(vao);

    auto& asset_shader = FLX_ASSET_GET(Asset::Shader, props.shader);
    asset_shader.Use();

    if (props.texture != "")
    {
      asset_shader.SetUniform_bool("u_use_texture", true);
      auto& asset_texture = FLX_ASSET_GET(Asset::Texture, props.texture);
      asset_texture.Bind(asset_shader, "u_texture", 0);
    }
    else if (props.color != Vector3::Zero)
    {
      asset_shader.SetUniform_bool("u_use_texture", false);
      asset_shader.SetUniform_vec3("u_color", props.color);
    }
    else
    {
      Log::Fatal("No texture or color specified for texture shader.");
    }

    asset_shader.SetUniform_vec3("u_color_to_add", props.color_to_add);
    asset_shader.SetUniform_vec3("u_color_to_multiply", props.color_to_multiply);

    // alignment
    Vector2 position = Vector2(props.position.x, props.position.y);
    switch (props.alignment)
    {
    case Renderer2DProps::Alignment_TopLeft:
      position += props.scale * 0.5f;
      break;
    case Renderer2DProps::Alignment_Center:
    default:
      break;
    }

    Matrix4x4 model = Matrix4x4::Identity;
    asset_shader.SetUniform_mat4("u_model", model.Translate(Vector3(-position.x, position.y, 0.0f)).Scale(Vector3(props.scale.x, props.scale.y, 1.0f)));
    static const Matrix4x4 view_matrix = Matrix4x4::LookAt(Vector3::Zero, Vector3::Forward, Vector3::Up);
    Matrix4x4 projection_view = Matrix4x4::Orthographic(
      0.0f, props.window_size.x,
      props.window_size.y, 0.0f,
      -2.0f, 2.0f
    ) * view_matrix;
    asset_shader.SetUniform_mat4("u_projection_view", projection_view);

    // draw
    glDrawArrays(GL_TRIANGLES, 0, 6);
    m_draw_calls++;

    glBindVertexArray(0);
  }

  void OpenGLSpriteRenderer::DrawPostProcessingLayer()
  {
      // Step 1: Full-screen quad vertices for post-processing (unchanged)
      static const float vertices[] = {
          -0.5f, -0.5f, 0.0f,   1.0f, 0.0f,
           0.5f, -0.5f, 0.0f,   0.0f, 0.0f,
           0.5f,  0.5f, 0.0f,   0.0f, 1.0f,
           0.5f,  0.5f, 0.0f,   0.0f, 1.0f,
          -0.5f,  0.5f, 0.0f,   1.0f, 1.0f,
          -0.5f, -0.5f, 0.0f,   1.0f, 0.0f
      };

      // Step 2: Render the texture (offscreen rendering)
      glBindFramebuffer(GL_FRAMEBUFFER, m_postProcessingFBO);

      // Step 3: Brightness Pass - Extract bright areas
      auto& brightness_shader = FLX_ASSET_GET(Asset::Shader, R"(/shaders/brightness)");
      brightness_shader.Use();
      brightness_shader.SetUniform_float("u_Threshold", 0.55f);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, bloomTexture); // Texture rendered in the previous step (scene texture)
      brightness_shader.SetUniform_int("scene", 0);

      // Render brightness to first ping-pong buffer
      glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO[0]); // Use ping-pong framebuffer for blur
      glBindVertexArray(m_rectVAO);
      glDrawArrays(GL_TRIANGLES, 0, 6);
      m_draw_calls++;

      // Step 4: Gaussian Blur Pass
     /* auto& blur_shader = FLX_ASSET_GET(Asset::Shader, R"(/shaders/blur)");
      blur_shader.Use();

      bool horizontal = true;
      int blur_passes = 2;

      for (int i = 0; i < blur_passes; ++i)
      {
          glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO[horizontal]);
          blur_shader.SetUniform_int("horizontal", horizontal);

          if (i == 0) {
              glBindTexture(GL_TEXTURE_2D, m_pingpongBuffer[0]);
          }
          else {
              glBindTexture(GL_TEXTURE_2D, m_pingpongBuffer[!horizontal]);
          }

          glBindVertexArray(m_rectVAO);
          glDrawArrays(GL_TRIANGLES, 0, 6);
          m_draw_calls++;
          horizontal = !horizontal;
      }*/

          // Step 5: Final Composition
          glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind default framebuffer

          //Not working
          
              auto& final_shader = FLX_ASSET_GET(Asset::Shader, R"(/shaders/finalComposite)");
              final_shader.Use();
              
              //glActiveTexture(GL_TEXTURE0);
              //glBindTexture(GL_TEXTURE_2D, postProcessingTexture); // Original scene texture
              //final_shader.SetUniform_int("screenTexture", 0);
              //
              //glActiveTexture(GL_TEXTURE1);
              //glBindTexture(GL_TEXTURE_2D, m_pingpongBuffer[/*!horizontal*/1]); // Final blurred texture
              //final_shader.SetUniform_int("bloomTexture", 1);
          


          glBindVertexArray(m_rectVAO);
          glDrawArrays(GL_TRIANGLES, 0, 6);
          m_draw_calls++;
      //}

      // Clean-up
      glBindVertexArray(0);
  }

  //Not used
  void OpenGLSpriteRenderer::ApplyBloomEffect()
  {
      bool horizontal = true, first_iteration = true;
      int amount = 10;  // Number of blur passes
      auto& blurHorizontalShader = FLX_ASSET_GET(Asset::Shader, AssetKey(R"(\shader\bloom\bloom_blur_horizontal.frag)"));
      auto& blurVerticalShader = FLX_ASSET_GET(Asset::Shader, AssetKey(R"(\shader\bloom\bloom_blur_vertical.frag)"));

      for (int i = 0; i < amount; i++)
      {
          glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO[horizontal]);
          if (horizontal)
              blurHorizontalShader.Use();
          else
              blurVerticalShader.Use();

          glBindTexture(GL_TEXTURE_2D, first_iteration ? m_brightBuffer : m_pingpongBuffer[!horizontal]);  // First iteration uses brightness buffer
          glDrawArrays(GL_TRIANGLES, 0, 6);

          horizontal = !horizontal;
          if (first_iteration) first_iteration = false;
      }

      glBindFramebuffer(GL_FRAMEBUFFER, 0);  // Back to default framebuffer

      // 3. Combine original scene with blurred bright areas
      auto& finalShader = FLX_ASSET_GET(Asset::Shader, AssetKey(R"(\shader\bloom\bloom_final_composite.frag)"));
      finalShader.Use();
      glDrawArrays(GL_TRIANGLES, 0, 6);  // Composite final scene with bloom
  }
}

//Not used
//{
    //// 1. Render to Framebuffer (offscreen rendering)
    //glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    //ClearFrameBuffer();  // Clear color and depth buffers

    //// Ensure the VAO and shader are bound
    //static GLuint vao = 0, vbo = 0;
    //if (vao == 0)
    //{
    //    glGenVertexArrays(1, &vao);
    //    glGenBuffers(1, &vbo);

    //    glBindVertexArray(vao);
    //    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    //    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //    glEnableVertexAttribArray(0);
    //    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    //    glEnableVertexAttribArray(1);
    //    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    //    glBindVertexArray(0);

    //    // free in freequeue
    //    FreeQueue::Push(
    //      [=]()
    //    {
    //        glDeleteBuffers(1, &vbo);
    //        glDeleteVertexArrays(1, &vao);
    //    }
    //    );
    //}

    //// guard
    //if (vao == 0 || props.shader == "" || props.scale == Vector2::Zero) return;

    //glBindVertexArray(vao);
    //auto& asset_shader = FLX_ASSET_GET(Asset::Shader, props.shader);
    //asset_shader.Use();

    //// Bind texture or color
    //if (!props.texture.empty()) {
    //    asset_shader.SetUniform_bool("u_use_texture", true);
    //    auto& asset_texture = FLX_ASSET_GET(Asset::Texture, props.texture);
    //    asset_texture.Bind(asset_shader, "u_texture", 0);
    //}
    //else if (props.color != Vector3::Zero) {
    //    asset_shader.SetUniform_bool("u_use_texture", false);
    //    asset_shader.SetUniform_vec3("u_color", props.color);
    //}
    //else {
    //    Log::Fatal("No texture or color specified for shader.");
    //}

    //// Set additional uniforms (e.g., color to add/multiply)
    //asset_shader.SetUniform_vec3("u_color_to_add", props.color_to_add);
    //asset_shader.SetUniform_vec3("u_color_to_multiply", props.color_to_multiply);

    //// Set up alignment and transformations
    //Vector2 position = props.position;
    //if (props.alignment == Renderer2DProps::Alignment_TopLeft) {
    //    position += props.scale * 0.5f;
    //}
    //Matrix4x4 model = Matrix4x4::Identity;
    //model = model.Translate(Vector3(-position.x, position.y, 0.0f)).Scale(Vector3(props.scale.x, props.scale.y, 1.0f));
    //asset_shader.SetUniform_mat4("u_model", model);

    //// Set projection and view matrices
    //static const Matrix4x4 view_matrix = Matrix4x4::LookAt(Vector3::Zero, Vector3::Forward, Vector3::Up);
    //Matrix4x4 projection_view = Matrix4x4::Orthographic(0.0f, props.window_size.x, props.window_size.y, 0.0f, -2.0f, 2.0f) * view_matrix;
    //asset_shader.SetUniform_mat4("u_projection_view", projection_view);

    //// Draw the sprite
    //glDrawArrays(GL_TRIANGLES, 0, 6);
    //m_draw_calls++;

    //// 2. Apply Bloom Effect
    //ApplyBloomEffect();  // Perform bright extraction and Gaussian blur

    //// 3. Bind back to default framebuffer and apply final composition
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //auto& finalShader = FLX_ASSET_GET(Asset::Shader, AssetKey(R"(\shader\bloom\bloom_final_composite.frag)"));
    //finalShader.Use();

    //// Bind the texture and the bloom texture (blurred bright areas)
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, m_colorBuffer);  // Original scene texture
    //finalShader.SetUniform_int("scene", 0);

    //glActiveTexture(GL_TEXTURE1);
    //glBindTexture(GL_TEXTURE_2D, m_pingpongBuffer[1]);  // Blurred bloom texture
    //finalShader.SetUniform_int("bloomBlur", 1);

    //// Final draw combining original texture and bloom effect
    //glDrawArrays(GL_TRIANGLES, 0, 6);

    //// Unbind the VAO
    //glBindVertexArray(0);
//  }
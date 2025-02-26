void APIENTRY glDebugOutput(GLenum source, 
                            GLenum type, 
                            unsigned int id, 
                            GLenum severity, 
                            GLsizei length, 
                            const char *message, 
                            const void *userParam)
{
	if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

	printf("---------------\n");
	printf("Debug Message (%u): %s\n", id, message);

	printf("Source:");
	switch (source)
	{
		case GL_DEBUG_SOURCE_API:
		{
			printf("api");
		}break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		{
			printf("Window system");
		}break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
		{
			printf("Shader compiler");
		}break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:
		{
			printf("third party");
		}break;
		case GL_DEBUG_SOURCE_APPLICATION:
		{
			printf("application");
		}break;
		case GL_DEBUG_SOURCE_OTHER:
		{
			printf("other");
		}break;
		default:
		{
			INVALID_CODE_PATH();
		}
	}

	printf("\n");

	printf("Type:");
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
	{
		printf("Error");
	}break;;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
	{
		printf("Deprecated behaviour");
	}break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  
	{
		printf("undefined behaviour");
	}break;
	case GL_DEBUG_TYPE_PORTABILITY:
	{
		printf("portability");
	}break;
	case GL_DEBUG_TYPE_PERFORMANCE:
	{
		printf("performance");
	}break;
	case GL_DEBUG_TYPE_MARKER:
	{
		printf("marker");
	}break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
	{
		printf("push group");
	}break;
	case GL_DEBUG_TYPE_POP_GROUP:
	{
		printf("pop group");
	}break;
	case GL_DEBUG_TYPE_OTHER:
	{
		printf("other");
	}break;
	default:
	{
		INVALID_CODE_PATH();
	}break;
	}
	printf("\n");

	printf("severity: ");
	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:
		{
			printf("high");
		}break;
		case GL_DEBUG_SEVERITY_MEDIUM:
		{
			printf("medium");
		}break;
		case GL_DEBUG_SEVERITY_LOW:
		{
			printf("low");
		}break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
		{
			printf("notification");
		}break;
	}
	printf("\n");
}

void check_compile_errors(GLuint shader, const char *type)
{
	int success;
	char infoLog[1024];

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 1024, 0, infoLog);
		printf("\n%s compilation error:\n%s\n", type, infoLog);
		INVALID_CODE_PATH();
	}
}

void check_link_errors(GLuint shader, const char *type)
{
	int success;
	char infoLog[1024];
	glGetProgramiv(shader, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shader, 1024, 0, infoLog);
		printf("\n%s linking error:\n%s\n", type, infoLog);
		INVALID_CODE_PATH();
	}
}

GLuint r_opengl_make_shader_program(char *vertexShaderSource, char *fragmentShaderSource)
{
	GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert_shader, 1, &vertexShaderSource, 0);
	glCompileShader(vert_shader);
	check_compile_errors(vert_shader, "vertex shader");

	GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag_shader, 1, &fragmentShaderSource, 0);
	glCompileShader(frag_shader);
	check_compile_errors(frag_shader, "fragment shader");

	GLuint shader_prog = glCreateProgram();
	glAttachShader(shader_prog, vert_shader);
	glAttachShader(shader_prog, frag_shader);

	glLinkProgram(shader_prog);
	check_link_errors(shader_prog, "vert/frag shader");

	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);

	return shader_prog;
}

GLuint r_opengl_make_buffer(size_t size)
{
	GLuint ssbo = 0;

	// use discard buffer?

	glCreateBuffers(1, &ssbo);
	glNamedBufferData(ssbo, size, 0, GL_STREAM_COPY_ARB);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

	return ssbo;
}

void r_opengl_init()
{
	GLuint default_rubbish_bs_vao;
	glCreateVertexArrays(1,&default_rubbish_bs_vao);
	glBindVertexArray(default_rubbish_bs_vao);
  
#if R_DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
	glDebugMessageCallback(glDebugOutput, 0);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
#endif
  
	r_opengl_state.shader_prog[R_OPENGL_SHADER_PROG_UI] = r_opengl_make_shader_program(r_vs_ui_src, r_fs_ui_src);
	r_opengl_state.inst_buffer[R_OPENGL_INST_BUFFER_UI] = r_opengl_make_buffer(Megabytes(8));
}

R_Handle r_opengl_alloc_texture(void *data, i32 w, i32 h, i32 n, R_Texture_params *p)
{
	R_Handle out = {};

	GLuint texture;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture);

	local_persist GLenum filter_table[R_TEXTURE_FILTER_COUNT] = 
	{
		GL_NEAREST,
		GL_LINEAR,
	};

	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, filter_table[p->min]);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, filter_table[p->max]);

	local_persist GLenum wrap_table[R_TEXTURE_WRAP_COUNT] = 
	{
		GL_CLAMP_TO_BORDER,
		GL_REPEAT,
	};

	glTextureParameteri(texture, GL_TEXTURE_WRAP_S, wrap_table[p->wrap]);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_T, wrap_table[p->wrap]);

	glTextureStorage2D(texture, 1, GL_SRGB8_ALPHA8, w, h);

	if(data)
	{
		GLenum channels = GL_RGBA;

		if(n == 3)
		{
			channels = GL_RGB;
		}

		glTextureSubImage2D(texture, 0, 0, 0, w, h, channels, GL_UNSIGNED_BYTE, data);
	}

	GLuint64 gpu_handle = glGetTextureHandleARB(texture);
	glMakeTextureHandleResidentARB(gpu_handle);

	out.u64_m[0] = gpu_handle;

	return out;
}

void r_opengl_submit(R_Pass_list *list, v2i win_size)
{
	f32 color[3] = {1,0,1};
	glClearBufferfv(GL_COLOR, 0, color);

	glViewport(0, 0, win_size.x, win_size.y);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	R_Pass_node *node = list->first;
	for(u32 i = 0; i < list->num; i ++)
	{
		R_Pass *pass = &node->pass;

		switch(pass->kind)
		{
			default:{}break;
			case R_PASS_KIND_UI:
			{
				R_Rect_pass rect_pass = pass->rect_pass;
				R_Batch_list *batches = &pass->rect_pass.rects;
				R_Batch *batch = batches->first;

				for(u32 j = 0; j < batches->num; j++)
				{
					void *ssbo_data = glMapNamedBufferRange(r_opengl_state.inst_buffer[R_OPENGL_INST_BUFFER_UI], 0, sizeof(m4f) + batch->count * sizeof(R_Rect), GL_MAP_WRITE_BIT | 
										GL_MAP_INVALIDATE_BUFFER_BIT);
					glUseProgram(r_opengl_state.shader_prog[R_OPENGL_SHADER_PROG_UI]);

					memcpy(ssbo_data, &rect_pass.proj_view, sizeof(m4f));

					memcpy((u8*)ssbo_data + sizeof(m4f), batch->base, batch->count * sizeof(R_Rect));


					glUnmapNamedBuffer(r_opengl_state.inst_buffer[R_OPENGL_INST_BUFFER_UI]);
					glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, sprite_draw_indices, batch->count);
					batch = batch->next;
				}
			}break;
		}

		node = node->next;
	}
}
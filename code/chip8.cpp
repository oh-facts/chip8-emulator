b32 is_chip_key_down(Input *input, u8 key)
{
	local_persist u32 key_table[KEY_COUNT] = {};
	local_persist b32 initialized = 0;
	
	if(!initialized)
	{
		initialized = 1;
		key_table[0x0] = 'x';
		key_table[0x1] = '1';
		key_table[0x2] = '2';
		key_table[0x3] = '3';
		key_table[0x4] = 'q';
		key_table[0x5] = 'w';
		key_table[0x6] = 'e';
		key_table[0x7] = 'a';
		key_table[0x8] = 's';
		key_table[0x9] = 'd';
		key_table[0xA] = 'z';
		key_table[0xB] = 'c';
		key_table[0xC] = '4';
		key_table[0xD] = 'r';
		key_table[0xE] = 'f';
		key_table[0xF] = 'v';
	}
	
	return input->keys[key_table[key]];
}

u8 *chip_get_vx(Chip8 *chip)
{
	u8 x = (chip->opcode & 0x0F00) >> 8;
	
	u8 *out = chip->reg + x;
	
	return out;
}

u8 *chip_get_vy(Chip8 *chip)
{
	u8 y = (chip->opcode & 0x00F0) >> 4;
	
	u8 *out = chip->reg + y;
	return out;
}

void chip_reload_rom(Chip8 *chip)
{
	u8 *start = (((u8*)chip) + CHIP_8_PERM_SIZE);
	memset(start, 0, sizeof(Chip8) - CHIP_8_PERM_SIZE);
	srand(time(0));
	chip->status = ChipStatus_Off;
}

void chip_run(Chip8 *chip, UI_Context *cxt, D_Bucket *draw, f32 delta)
{
	switch(chip->status)
	{
		default: {INVALID_CODE_PATH();}break;
		case ChipStatus_Null:{}break;
		
		case ChipStatus_Off:
		{
			chip->inst_per_step = 1;
			
			for(i32 i = 0; i < 80; ++i)
			{
				chip->ram[i] = chip8_fontset[i];	
			}
			chip->status = ChipStatus_Hot;
		}break;
		
		case ChipStatus_Hot:
		{
			Arena_temp temp = scratch_begin(0,0);
			File_data rom = read_file(temp.arena, chip->rom_path, FILE_TYPE_BINARY);
			
			mem_cpy(&chip->ram[CHIP8_PROG_START], rom.bytes, rom.size);
			
			chip->pc = CHIP8_PROG_START;
			chip->status = ChipStatus_Active;
			scratch_end(&temp);
		}break;
		
		case ChipStatus_Active:
		{
			for(u32 i = 0; i < chip->inst_per_step; i++)
			{
				u16 opcode = chip->ram[chip->pc] << 8 | chip->ram[chip->pc + 1];
				chip->opcode = opcode;
				
				switch(opcode & 0xF000)
				{
					default: INVALID_CODE_PATH();
					case 0x0000:
					{
						switch(opcode & 0x0FFF)
						{
							// 0NNN
							default:
							{
								u16 addr = opcode & 0x0FFF;
								chip->pc = addr;
							}break;
							case 0x00E0:
							{
								// cls
								memset(chip->display, 0, 64 * 32);
								chip->pc += 2;
							}break;
							case 0x00EE:
							{
								//ret
								chip->sp--;
								chip->pc = chip->stack[chip->sp];
								chip->pc += 2;
							}break;
						}
					}break;
					
					case 0x1000:
					{
						//jp
						u16 addr = opcode & 0x0FFF;
						
						chip->pc = addr;
					}break;
					
					case 0x2000:
					{
						// call
						
						u16 addr = opcode & 0x0FFF;
						
						chip->stack[chip->sp] = chip->pc;
						chip->sp ++;
						chip->pc = addr;
					}break;
					
					case 0x3000:
					{
						// SE r,b
						u8 val = (opcode & 0x00FF);
						
						if(*chip_get_vx(chip) == val)
						{
							chip->pc += 2;
						}
						
						chip->pc += 2;
					}break;
					
					case 0x4000:
					{
						// SNE
						u8 val = (opcode & 0x00FF);
						
						if(*chip_get_vx(chip) != val)
						{
							chip->pc += 2;
						}
						
						chip->pc += 2;
						
					}break;
					
					case 0x5000:
					{
						// 5xy0 - SE Vx, Vy
						
						if(*chip_get_vx(chip) == *chip_get_vy(chip))
						{
							chip->pc += 2;
						}
						
						chip->pc += 2;
						
					}break;
					
					//LD
					case 0x6000:
					{
						u8 val = opcode & 0x00FF;
						*chip_get_vx(chip) = val;
						
						chip->pc += 2;
					}break;
					
					case 0x7000:
					{
						u8 val = (opcode & 0x00FF);
						
						*chip_get_vx(chip) += val;
						
						chip->pc += 2;
					}break;
					
					case 0x8000:
					{
						switch(opcode & 0x000F)
						{
							default: INVALID_CODE_PATH();
							case 0x0000:
							{
								*chip_get_vx(chip) = *chip_get_vy(chip);
								
								chip->pc += 2;
							}break;
							
							case 0x0001:
							{
								*chip_get_vx(chip) |= *chip_get_vy(chip);
								
								chip->pc += 2;
							}break;
							
							case 0x0002:
							{
								*chip_get_vx(chip) &= *chip_get_vy(chip);
								
								chip->pc += 2;
							}break;
							
							case 0x0003:
							{
								*chip_get_vx(chip) ^= *chip_get_vy(chip);
								chip->pc += 2;
							}break;
							
							// 8xy4 - ADD Vx, Vy
							case 0x0004:
							{
								u16 sum = *chip_get_vx(chip) + *chip_get_vy(chip);
								
								if(sum > 0xFF)
								{
									chip->reg[0xF] = 1;
								}
								else
								{
									chip->reg[0xF] = 0;
								}
								u8 sum_u8 = sum & 0x00FF;
								*chip_get_vx(chip) = sum_u8;
								
								chip->pc += 2;
							}break;
							
							//8xy5 - SUB Vx, Vy
							case 0x0005:
							{
								if(*chip_get_vx(chip) < *chip_get_vy(chip))
								{
									chip->reg[0xF] = 0;
								}
								else
								{
									chip->reg[0xF] = 1;
								}
								
								*chip_get_vx(chip) -= *chip_get_vy(chip);
								
								chip->pc += 2;
							}break;
							
							case 0x0006:
							{
								chip->reg[0xF] = *chip_get_vx(chip) & 0x1;
								*chip_get_vx(chip) >>= 1;
								
								chip->pc += 2;
							}break;
							
							case 0x0007:
							{
								
								if(*chip_get_vx(chip) > *chip_get_vy(chip))
								{
									chip->reg[0xF] = 0;
								}
								else
								{
									chip->reg[0xF] = 1;
								}
								
								*chip_get_vx(chip) = *chip_get_vy(chip) - *chip_get_vx(chip);
								
								chip->pc += 2;
								
							}break;
							
							case 0x000E:
							{
								chip->reg[0xF] = *chip_get_vx(chip) >> 7;
								*chip_get_vx(chip) <<= 1;
								
								chip->pc += 2;
							}break;
							
						}
						
					}break;
					
					case 0x9000:
					{
						
						if(*chip_get_vx(chip) != *chip_get_vy(chip))
						{
							chip->pc += 2;
						}
						
						chip->pc += 2;
						
					}break;
					
					case 0xA000:
					{
						// LD I, addr
						u16 val = opcode & 0x0FFF;
						chip->I = val;
						
						chip->pc += 2;
					}break;
					
					case 0xB000:
					{
						chip->pc = chip->reg[0x0] + opcode & 0x0FFF;
					}break;
					
					// rnd
					case 0xC000:
					{
						u8 rnd = rand() % 256;
						
						u8 val = (opcode & 0x00FF);
						
						*chip_get_vx(chip) = rnd & val;
						
						chip->pc += 2;
					}break;
					
					// drw
					case 0xD000:
					{
						u8 pos_x = *chip_get_vx(chip) % 64;
						u8 pos_y = *chip_get_vy(chip) % 32;
						u8 n = (opcode & 0x000F);
						
						u8 *sprite = chip->ram + chip->I;
						chip->reg[0xF] = 0;
						
						for (int i = 0; i < n; i++)
						{
							for (int j = 0; j < 8; j++)
							{
								int val = (sprite[i] >> (7 - j)) & 1;
								int scrptr = (64 * ((pos_y + i) % 32)) + ((pos_x + j) % 64);
								if (chip->display[scrptr] && val)
								{
									chip->reg[0xF] = 1;
								}
								chip->display[scrptr] ^= val;
							}
						}
						
						chip->pc += 2;
					} break;
					
					case 0xE000:
					{
						
						if((opcode & 0x00F0) == 0x0090)
						{
							u8 key = *chip_get_vx(chip);
							
							if(is_chip_key_down(g_input, key))
							{
								chip->pc += 2;
							}
							
							chip->pc += 2;
						}
						else if((opcode & 0x00F0) == 0x00A0)
						{
							u8 key = *chip_get_vx(chip);
							if(!is_chip_key_down(g_input, key))
							{
								chip->pc += 2;
							}
							
							chip->pc += 2;
						}
						else
						{
							INVALID_CODE_PATH();
						}
					}break;
					
					case 0xF000:
					{
						switch(opcode & 0x00FF)
						{
							default: INVALID_CODE_PATH();
							case 0x0007:
							{
								*chip_get_vx(chip) = chip->delay_t;
								
								chip->pc += 2;
							}break;
							
							case 0x000A:
							{
								
								for(u32 i = 0; i < 16; i++)
								{
									if(is_chip_key_down(g_input, i))
									{
										*chip_get_vx(chip) = i; 
										chip->pc += 2;
										break;
									}
									
								}
								
							}break;
							
							//Fx15
							case 0x0015:
							{
								chip->delay_t = *chip_get_vx(chip);
								
								chip->pc += 2;
							}break;
							
							case 0x0018:
							{
								chip->sound_t = *chip_get_vx(chip);
								chip->pc += 2;
							}break;
							
							//Fx1E - ADD I, Vx
							case 0x001E:
							{
								chip->I += *chip_get_vx(chip);
								
								chip->pc += 2;
							}break;
							
							case 0x0029:
							{
								chip->I = (*chip_get_vx(chip)) * 0x5;
								chip->pc += 2;
							}break;
							
							case 0x0033:
							{
								u8 val = *chip_get_vx(chip);
								chip->ram[chip->I] = val / 100;
								chip->ram[chip->I + 1] = (val / 10) % 10;
								chip->ram[chip->I + 2] = val % 10;
								
								chip->pc += 2;
								
							}break;
							
							//FX55
							case 0x0055:
							{
								u8 index = (opcode & 0x0F00) >> 8;
								for(u32 i = 0; i <= index; i++)
								{
									chip->ram[chip->I + i] = chip->reg[i];
								}
								
								chip->pc += 2;
							}break;
							
							//FX65
							case 0x0065:
							{
								u8 index = (opcode & 0x0F00) >> 8;
								for(u32 i = 0; i <= index; i++)
								{
									chip->reg[i] = chip->ram[chip->I + i];
								}
								
								chip->pc += 2;
							}break;
							
						}
						
					}break;
					
				}
			}
			
			local_persist f32 timer_60hz = 0;
			timer_60hz += delta;
			// i am probably doing this incorrectly
			if(timer_60hz > 1/60.f)
			{
				if(chip->delay_t > 0)
				{
					chip->delay_t -= 1;
				}
				delta = 0;
			}
			f32 scale = 0.04;
			
			for(i32 x = 0; x < 64; x++)
			{
				for(i32 y = 0; y < 32; y++)
				{
					v4f color = {{}};
					
					if(chip->display[y * 64 + x] == 1)
					{
						color = CHIP8_COLOR_FG;
					}
					else
					{
						color = CHIP8_COLOR_BG;
					}
					
					d_draw_rect(draw, v2f{{(x - 64.f * 0.5f) * scale, (32.f - y - 32.f * 0.5f)*scale}}, v2f{{scale,scale}}, color);
				}
			}
			
			
		}break;
		
	}

	if(chip->dialog_data.completed)
	{
		if(chip->dialog_data.path.len > 0)
		{
			u8 *start = (((u8*)chip) + CHIP_8_PERM_SIZE);
			memset(start, 0, sizeof(Chip8) - CHIP_8_PERM_SIZE);
			mem_cpy(chip->rom_path, chip->dialog_data.path.c, chip->dialog_data.path.len);
			srand(time(0));
			chip->status = ChipStatus_Off;
		}

		chip->dialog_data.completed = 0;
	}
	
	ui_begin(cxt);
	
	ui_push_fixed_pos(cxt, v2f{{-1.3, 0.9}});
	ui_push_text_color(cxt, CHIP8_COLOR_TEXT);
	ui_colf(cxt, "col")
	{
		ui_push_size_kind(cxt, UI_SizeKind_TextContent);
		
		if(ui_labelf(cxt, "Load Rom").hot)
		{
			printf("hot\n");
			os_open_file_dialog(&chip->dialog_data);
		}
		
		if(ui_labelf(cxt, "%s", chip->rom_path).active)
		{
			chip_reload_rom(chip);
		}
		
		if(ui_labelf(cxt, "IPS: %lu", chip->inst_per_step).active)
		{
			chip->inst_per_step = 1;
		}
		
		ui_rowf(cxt, "row")
		{
			if(ui_labelf(cxt, "inc").active)
			{
				chip->inst_per_step ++;
				chip->inst_per_step = ClampTop(chip->inst_per_step, 10000);
			}
			if(ui_labelf(cxt, "dec").active)
			{
				chip->inst_per_step --;
				chip->inst_per_step = ClampTop(chip->inst_per_step, 10000);
			}
		}
		
		ui_pop_size_kind(cxt);
	}
	
	ui_pop_text_color(cxt);
	ui_pop_fixed_pos(cxt);
	ui_end(cxt);
}
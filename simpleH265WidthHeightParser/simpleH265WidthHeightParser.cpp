#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

typedef struct _nalbuf_t{
  uint8_t *buf;
  uint64_t pos;
  int bitpos;
} nalbuf_t;

typedef struct _sps_t{
  uint16_t forbidden_zero_bit;
  uint16_t nal_unit_type;
  uint16_t nuh_layer_id;
  uint16_t nuh_temporal_id_plus1;
  uint8_t sps_video_parameter_set_id;
  uint8_t sps_max_sub_layers_minus1;
  uint8_t sps_temporal_id_nesting_flag;
  int profile_tier_level;
  uint32_t sps_seq_parameter_set_id;
  uint32_t chroma_format_idc;
  uint8_t separate_colur_plane_flag;
  uint32_t pic_width_in_luma_samples;
  uint32_t pic_height_in_luma_samples;
} sps_t;

typedef struct _profile_tier_level_data_t
{
	uint8_t general_profile_space;//	u(2)
	uint8_t general_tier_flag;//	u(1)
	uint8_t general_profile_idc;//	u(5)
	//for( i = 0; i < 32; i++ )	
	uint8_t general_profile_compatibility_flag[32];//	u(1)
	uint8_t general_progressive_source_flag;//	u(1)
	uint8_t general_interlaced_source_flag;//	u(1)
	uint8_t general_non_packed_constraint_flag;//	u(1)
	uint8_t general_frame_only_constraint_flag;//	u(1)
//uint64_t general_reserved_zero_44bits;  //[Ed. (GJS): Adjust semantics accordingly.]	u(44)
//	if( general_profile_idc  = =  4  | |  general_profile_compatibility_flag[ 4 ]  | |
//			general_profile_idc  = =  5  | |  general_profile_compatibility_flag[ 5 ]  | |
//			general_profile_idc  = =  6  | |  general_profile_compatibility_flag[ 6 ]  | |
//			general_profile_idc  = =  7  | |  general_profile_compatibility_flag[ 7 ] ) {
//			/* The number of bits in this syntax structure is not affected by this condition */	
	uint8_t general_max_12bit_constraint_flag;//	u(1)
	uint8_t general_max_10bit_constraint_flag;//	u(1)
	uint8_t general_max_8bit_constraint_flag;//	u(1)
	uint8_t general_max_422chroma_constraint_flag;//	u(1)
	uint8_t general_max_420chroma_constraint_flag;//	u(1)
	uint8_t general_max_monochrome_constraint_flag;//u(1)
	uint8_t general_intra_constraint_flag;//	u(1)
	uint8_t general_one_picture_only_constraint_flag;//	u(1)
	uint8_t general_lower_bit_rate_constraint_flag;//	u(1)
	uint64_t general_reserved_zero_34bits;//	u(34)
//		} else	
	uint64_t	general_reserved_zero_43bits;//	u(43)
//		if( ( general_profile_idc  >=  1  &&  general_profile_idc  <=  5 )  | |
//			 general_profile_compatibility_flag[ 1 ]  | |  general_profile_compatibility_flag[ 2 ]  | |
//			 general_profile_compatibility_flag[ 3 ]  | |  general_profile_compatibility_flag[ 4 ]  | |
//			 general_profile_compatibility_flag[ 5 ] )
//			/* The number of bits in this syntax structure is not affected by this condition */	
	uint8_t general_inbld_flag;//	u(1)
//		else	
	uint8_t general_reserved_zero_bit;//	u(1)

	uint8_t general_level_idc;//	u(8)
	//for( i = 0; i < maxNumSubLayersMinus1; i++ ) value of sps_max_sub_layers_minus1 shall be in the range of 0 to 6, inclusive
	uint8_t sub_layer_profile_present_flag[8];//	u(1)
	uint8_t sub_layer_level_present_flag[8];//	u(1)
	//if( maxNumSubLayersMinus1 > 0 )	
	//	for( i = maxNumSubLayersMinus1; i < 8; i++ )	
	uint8_t reserved_zero_2bits[2];//	u(2)
	//for( i = 0; i < maxNumSubLayersMinus1; i++ ) {	
		//if( sub_layer_profile_present_flag[ i ] ) {	
	uint8_t sub_layer_profile_space[7];//	u(2)
	uint8_t sub_layer_tier_flag[7];//	u(1)
	uint8_t sub_layer_profile_idc[7];//	u(5)
		    //for( j = 0; j < 32; j++ )	
	uint8_t sub_layer_profile_compatibility_flag[7][32];//	u(1)
	uint8_t sub_layer_progressive_source_flag[7];//	u(1)
	uint8_t sub_layer_interlaced_source_flag[7];//	u(1)
	uint8_t sub_layer_non_packed_constraint_flag[7];//	u(1)
	uint8_t sub_layer_frame_only_constraint_flag[7];//	u(1)
//	uint64_t sub_layer_reserved_zero_44bits[7];//	u(44)
//				if( sub_layer_profile_idc[ i ]  = =  4  | |  sub_layer_profile_compatibility_flag[ i ][ 4 ]  | |
//				sub_layer_profile_idc[ i ]  = =  5  | |  sub_layer_profile_compatibility_flag[ i ][ 5 ]  | |
//				sub_layer_profile_idc[ i ]  = =  6  | |  sub_layer_profile_compatibility_flag[ i ][ 6 ]  | |
//				sub_layer_profile_idc[ i ]  = =  7  | |  sub_layer_profile_compatibility_flag[ i ][ 7 ] ) {
//				/* The number of bits in this syntax structure is not affected by this condition */	
	uint8_t sub_layer_max_12bit_constraint_flag[7];//	u(1)
	uint8_t sub_layer_max_10bit_constraint_flag[7];//	u(1)
	uint8_t sub_layer_max_8bit_constraint_flag[7];//	u(1)
  uint8_t sub_layer_max_422chroma_constraint_flag[7];//	u(1)
  uint8_t sub_layer_max_420chroma_constraint_flag[7];//	u(1)
	uint8_t sub_layer_max_monochrome_constraint_flag[7];//	u(1)
	uint8_t sub_layer_intra_constraint_flag[7];//	u(1)
	uint8_t sub_layer_one_picture_only_constraint_flag[7];//	u(1)
	uint8_t sub_layer_lower_bit_rate_constraint_flag[7];//	u(1)
	uint64_t sub_layer_reserved_zero_34bits[7];//	u(34)
//	} else	
	uint64_t sub_layer_reserved_zero_43bits[7];//	u(43)
//	if( ( sub_layer_profile_idc[ i ]  >=  1  &&  sub_layer_profile_idc[ i ]  <=  5 )  | |
//	 sub_layer_profile_compatibility_flag[ 1 ]  | |
//	 sub_layer_profile_compatibility_flag[ 2 ]  | |
//	 sub_layer_profile_compatibility_flag[ 3 ]  | |
//	 sub_layer_profile_compatibility_flag[ 4 ]  | |
//	 sub_layer_profile_compatibility_flag[ 5 ] )
	/* The number of bits in this syntax structure is not affected by this condition */	
	uint8_t sub_layer_inbld_flag[7];//	u(1)
//	else	
	uint8_t sub_layer_reserved_zero_bit[7];//	u(1)
	//if( sub_layer_level_present_flag[ i ] )	
	uint8_t sub_layer_level_idc[7];//	u(8)
} profile_tier_level_data_t;


uint32_t read_bits(nalbuf_t *nal, int nbits);
uint8_t read_bit(nalbuf_t *nal);
uint64_t read_bits64(nalbuf_t *nal, int nbits);
uint32_t read_uev(nalbuf_t *nal);

profile_tier_level_data_t *profile_tier_level_data = NULL;
uint32_t read_bits(nalbuf_t *nal, int nbits)
{
  uint32_t ret = 0;
  for(int i = 0; i < nbits; i++){
    ret = (ret << 1) | read_bit(nal);
  }
  return ret;
}

uint8_t read_bit(nalbuf_t *nal)
{
  if(nal->bitpos == 0){
    nal->pos++;
    nal->bitpos = 8;
  }

  printf("bit pos: %d, pos: %lu\n", nal->bitpos, nal->pos);
  uint8_t ret = nal->buf[nal->pos] & (1 << (nal->bitpos -1));
  nal->bitpos --;
  printf("%d", ret > 0 ? 1: 0);
  return ret > 0 ? 1 : 0;
}

uint64_t read_bits64(nalbuf_t * nal, int nbits) {
	uint64_t ret = 0;
	for (int i = 0; i < nbits; i++) {
		ret = (ret << 1) | read_bit(nal);
	}
	return ret;
}

uint32_t read_uev(nalbuf_t *nal)
{
  printf("[");
  int zero_leading_bits = -1;
  uint8_t b = 0;
  for (b = 0; !b; zero_leading_bits++){
    b = read_bit(nal);
  }
  uint32_t ret = (1 << zero_leading_bits) - 1 + read_bits(nal, zero_leading_bits);
  printf("ret: %d\n", ret);
  printf("]");
  return ret;
}

void skip_profile_tier_level(nalbuf_t *pnal_buffer, int maxNumSubLayersMinus1)
{
  profile_tier_level_data->general_profile_space = (uint8_t)read_bits(pnal_buffer, 2);
  printf("general profile space: %d\n", profile_tier_level_data->general_profile_space);
	profile_tier_level_data->general_tier_flag = (uint8_t)read_bit(pnal_buffer);
  printf("general tier flag: %d\n", profile_tier_level_data->general_tier_flag);
	profile_tier_level_data->general_profile_idc = (uint8_t)read_bits(pnal_buffer, 5);
  printf("genral profile idc: %d\n", profile_tier_level_data->general_profile_idc);
	for (int j = 0; j < 32; j++)
	{
		profile_tier_level_data->general_profile_compatibility_flag[j] = (uint8_t)read_bit(pnal_buffer);
    printf("genral profile compatibility flag: %d\n", profile_tier_level_data->general_profile_compatibility_flag[j]);
	}
	profile_tier_level_data->general_progressive_source_flag = (uint8_t)read_bit(pnal_buffer);
  printf("genral progressive source flag: %d\n", profile_tier_level_data->general_progressive_source_flag);
	profile_tier_level_data->general_interlaced_source_flag = (uint8_t)read_bit(pnal_buffer);
  printf("general interlaced source flag: %d\n", profile_tier_level_data->general_interlaced_source_flag);
	profile_tier_level_data->general_non_packed_constraint_flag = (uint8_t)read_bit(pnal_buffer);
  printf("general non packed constraint flag: %d\n", profile_tier_level_data->general_non_packed_constraint_flag);
	profile_tier_level_data->general_frame_only_constraint_flag = (uint8_t)read_bit(pnal_buffer);
  printf("general frame constraint flag: %d\n", profile_tier_level_data->general_frame_only_constraint_flag);
	//profile_tier_level_data->general_reserved_zero_44bits = (uint64_t)read_bits64(pnal_buffer, 44);
	if (profile_tier_level_data->general_profile_idc == 4 || profile_tier_level_data->general_profile_compatibility_flag[4] ||
		profile_tier_level_data->general_profile_idc == 5 || profile_tier_level_data->general_profile_compatibility_flag[5] ||
		profile_tier_level_data->general_profile_idc == 6 || profile_tier_level_data->general_profile_compatibility_flag[6] ||
		profile_tier_level_data->general_profile_idc == 7 || profile_tier_level_data->general_profile_compatibility_flag[7])
	/* The number of bits in this syntax structure is not affected by this condition */
	{
		profile_tier_level_data->general_max_12bit_constraint_flag = (uint8_t)read_bit(pnal_buffer);//	u(1)
		profile_tier_level_data->general_max_10bit_constraint_flag = (uint8_t)read_bit(pnal_buffer);//	u(1)
		profile_tier_level_data->general_max_8bit_constraint_flag = (uint8_t)read_bit(pnal_buffer);//	u(1)
		profile_tier_level_data->general_max_422chroma_constraint_flag = (uint8_t)read_bit(pnal_buffer);//	u(1)
		profile_tier_level_data->general_max_420chroma_constraint_flag = (uint8_t)read_bit(pnal_buffer);//	u(1)
		profile_tier_level_data->general_max_monochrome_constraint_flag = (uint8_t)read_bit(pnal_buffer);//	u(1)
		profile_tier_level_data->general_intra_constraint_flag = (uint8_t)read_bit(pnal_buffer);//	u(1)
		profile_tier_level_data->general_one_picture_only_constraint_flag = (uint8_t)read_bit(pnal_buffer);//	u(1)
		profile_tier_level_data->general_lower_bit_rate_constraint_flag = (uint8_t)read_bit(pnal_buffer);//	u(1)
		profile_tier_level_data->general_reserved_zero_34bits = (uint64_t)read_bits64(pnal_buffer, 34);//	u(34)
	}
	else
	{
		profile_tier_level_data->general_reserved_zero_43bits = (uint64_t)read_bits64(pnal_buffer, 43);//	u(43)
		if ((profile_tier_level_data->general_profile_idc >= 1 && profile_tier_level_data->general_profile_idc <= 5) ||
			profile_tier_level_data->general_profile_compatibility_flag[1] || profile_tier_level_data->general_profile_compatibility_flag[2] ||
			profile_tier_level_data->general_profile_compatibility_flag[3] || profile_tier_level_data->general_profile_compatibility_flag[4] ||
			profile_tier_level_data->general_profile_compatibility_flag[5])
		/* The number of bits in this syntax structure is not affected by this condition */
		{
			profile_tier_level_data->general_inbld_flag = (uint8_t)read_bit(pnal_buffer); //u(1)
      printf("general inbld flag: %d\n", profile_tier_level_data->general_inbld_flag);
		}
		else
		{
			profile_tier_level_data->general_reserved_zero_bit = (uint8_t)read_bit(pnal_buffer);//	u(1)
      printf("general reserved zero bit: %d\n", profile_tier_level_data->general_reserved_zero_bit);
		}
	}
	profile_tier_level_data->general_level_idc = (uint8_t)read_bits(pnal_buffer, 8);
  printf("general level idc: %d\n", profile_tier_level_data->general_level_idc);
	for (int i = 0; i < maxNumSubLayersMinus1; i++)
	{
		profile_tier_level_data->sub_layer_profile_present_flag[i] = (uint8_t)read_bit(pnal_buffer);
		profile_tier_level_data->sub_layer_level_present_flag[i] = (uint8_t)read_bit(pnal_buffer);
    printf("sub layer profile present flag: %d\n", profile_tier_level_data->sub_layer_profile_present_flag[i]);
    printf("sub layer level present flag: %d\n", profile_tier_level_data->sub_layer_level_present_flag[i]);
	}

	if (maxNumSubLayersMinus1 > 0)
	{
		for (int i = maxNumSubLayersMinus1; i < 8; i++)
		{
			profile_tier_level_data->reserved_zero_2bits[i] = (uint8_t)read_bits(pnal_buffer, 2);
		}
	}

  printf("maxNumSubLayersMinus1: %d\n", maxNumSubLayersMinus1);
	for (int i = 0; i < maxNumSubLayersMinus1; i++)
	{
		if (profile_tier_level_data->sub_layer_profile_present_flag[i])
		{
			profile_tier_level_data->sub_layer_profile_space[i] = (uint8_t)read_bits(pnal_buffer, 2);
			profile_tier_level_data->sub_layer_tier_flag[i] = (uint8_t)read_bit(pnal_buffer);
			profile_tier_level_data->sub_layer_profile_idc[i] = (uint8_t)read_bits(pnal_buffer, 5);
			for (int j = 0; j < 32; j++)
			{
				profile_tier_level_data->sub_layer_profile_compatibility_flag[i][j] = (uint8_t)read_bit(pnal_buffer);
			}
			profile_tier_level_data->sub_layer_progressive_source_flag[i] = read_bit(pnal_buffer);
			profile_tier_level_data->sub_layer_interlaced_source_flag[i] = read_bit(pnal_buffer);
			profile_tier_level_data->sub_layer_non_packed_constraint_flag[i] = read_bit(pnal_buffer);
			profile_tier_level_data->sub_layer_frame_only_constraint_flag[i] = read_bit(pnal_buffer);
//			profile_tier_level_data->sub_layer_reserved_zero_44bits[i] = (uint64_t)read_bits64(pnal_buffer, 44);

			if (profile_tier_level_data->sub_layer_profile_idc[i] == 4 ||
				profile_tier_level_data->sub_layer_profile_compatibility_flag[i][4] ||
				profile_tier_level_data->sub_layer_profile_idc[i] == 5 ||
				profile_tier_level_data->sub_layer_profile_compatibility_flag[i][5] ||
				profile_tier_level_data->sub_layer_profile_idc[i] == 6 ||
				profile_tier_level_data->sub_layer_profile_compatibility_flag[i][6] ||
				profile_tier_level_data->sub_layer_profile_idc[i] == 7 ||
				profile_tier_level_data->sub_layer_profile_compatibility_flag[i][7])
			{
				/* The number of bits in this syntax structure is not affected by this condition */
				profile_tier_level_data->sub_layer_max_12bit_constraint_flag[i] = (uint8_t)read_bit(pnal_buffer);//	u(1)
				profile_tier_level_data->sub_layer_max_10bit_constraint_flag[i] = (uint8_t)read_bit(pnal_buffer);//	u(1)
				profile_tier_level_data->sub_layer_max_8bit_constraint_flag[i] = (uint8_t)read_bit(pnal_buffer);//	u(1)
				profile_tier_level_data->sub_layer_max_422chroma_constraint_flag[i] = (uint8_t)read_bit(pnal_buffer);//	u(1)
				profile_tier_level_data->sub_layer_max_420chroma_constraint_flag[i] = (uint8_t)read_bit(pnal_buffer);//	u(1)
				profile_tier_level_data->sub_layer_max_monochrome_constraint_flag[i] = (uint8_t)read_bit(pnal_buffer);//	u(1)
				profile_tier_level_data->sub_layer_intra_constraint_flag[i] = (uint8_t)read_bit(pnal_buffer);//	u(1)
				profile_tier_level_data->sub_layer_one_picture_only_constraint_flag[i] = (uint8_t)read_bit(pnal_buffer);//	u(1)
				profile_tier_level_data->sub_layer_lower_bit_rate_constraint_flag[i] = (uint8_t)read_bit(pnal_buffer);//	u(1)
				profile_tier_level_data->sub_layer_reserved_zero_34bits[i] = (uint64_t)read_bits(pnal_buffer, 34);//	u(34)
			}
			else
			{
				profile_tier_level_data->sub_layer_reserved_zero_43bits[i] = read_bits64(pnal_buffer, 43);//	u(43)
			}
			if ((profile_tier_level_data->sub_layer_profile_idc[i] >= 1 && profile_tier_level_data->sub_layer_profile_idc[i] <= 5) ||
				profile_tier_level_data->sub_layer_profile_compatibility_flag[1] ||
				profile_tier_level_data->sub_layer_profile_compatibility_flag[2] ||
				profile_tier_level_data->sub_layer_profile_compatibility_flag[3] ||
				profile_tier_level_data->sub_layer_profile_compatibility_flag[4] ||
				profile_tier_level_data->sub_layer_profile_compatibility_flag[5])
			/* The number of bits in this syntax structure is not affected by this condition */
			{
				profile_tier_level_data->sub_layer_inbld_flag[i] = (uint8_t)read_bit(pnal_buffer);//		u(1)
			}
			else
			{
				profile_tier_level_data->sub_layer_reserved_zero_bit[i] = (uint8_t)read_bit(pnal_buffer);//		u(1)
			}
		}
    printf("sub layer level present flag: %d\n", profile_tier_level_data->sub_layer_level_present_flag[i]);
		if (profile_tier_level_data->sub_layer_level_present_flag[i])
		{
			profile_tier_level_data->sub_layer_level_idc[i] = read_bits(pnal_buffer, 8);
      printf("sub layer level idc:%d\n", profile_tier_level_data->sub_layer_level_idc[i]);
		}
	}
}

bool check_prevention_three_byte(uint8_t *cur_byte)
{
  nalbuf_t *nal_data = (nalbuf_t*)malloc(sizeof(nalbuf_t));
  nal_data->buf = cur_byte;
  nal_data->pos = -1;
  if(read_bits(nal_data, 24) == 0x000003){
    // printf("vaule is prevention three byte\n");
    return true;
  }
  free(nal_data);
  nal_data = NULL;
  return false;
}

void print_framedata(uint8_t* data, int data_len)
{
  printf("[");
    
  for(int i=0 ; (i<32)&&(i<data_len) ; i++)
    {
      if( (i>0) && (0==i%4) )
        printf("  %02X", data[i]);
      else
        printf("%02X", data[i]);
    }
    
  printf("]\n");
}

void process_nalu(uint8_t *payload, uint64_t payload_cnt, nalbuf_t *nal)
{
  uint8_t *check_payload = payload;

  nal->buf = (uint8_t*)malloc(payload_cnt);
  nal->pos = -1;

  uint8_t *begin = nal->buf;
  for(int i = 0; i < payload_cnt; i++){
    if(check_prevention_three_byte(check_payload)){
      memcpy(nal->buf, check_payload, 2);
      nal->buf = nal->buf + 2;
      check_payload = check_payload + 3;
    }
    else{
      memcpy(nal->buf, check_payload, 1);
      nal->buf = nal->buf + 1;
      check_payload = check_payload + 1;
    }
  }
  nal->buf = begin;
  print_framedata(nal->buf, 33);
}

void parse_sps(sps_t *sps, nalbuf_t *nal){
  read_bits(nal, 32);
  sps->forbidden_zero_bit |= (uint16_t)read_bits(nal, 1);
  printf("forbidden bit: %d, pos: %d\n", sps->forbidden_zero_bit, nal->pos);
  sps->nal_unit_type |= (uint16_t)read_bits(nal, 6);
  printf("nal unit type: %d, pos: %lu\n", sps->nal_unit_type, nal->pos);
  sps->nuh_layer_id |= (uint16_t)read_bits(nal, 6);
  sps->nuh_temporal_id_plus1 |= (uint16_t)read_bits(nal, 3);
  sps->sps_video_parameter_set_id = (uint8_t)read_bits(nal, 4);
  sps->sps_max_sub_layers_minus1 = (uint8_t)read_bits(nal, 3);
  printf("max sub layers: %d\n", sps->sps_max_sub_layers_minus1);
  sps->sps_temporal_id_nesting_flag = (uint8_t)read_bit(nal);
  skip_profile_tier_level(nal, sps->sps_max_sub_layers_minus1);
  // read_bits(nal, 24);
  sps->sps_seq_parameter_set_id = read_uev(nal);
  sps->chroma_format_idc = read_uev(nal);
  if(sps->chroma_format_idc == 3){
    printf("chroma format idc is 3\n");
    sps->separate_colur_plane_flag = (uint8_t)read_bit(nal);
  }
  sps->pic_width_in_luma_samples = read_uev(nal);
  printf("width:%d\n", sps->pic_width_in_luma_samples);
  sps->pic_height_in_luma_samples = read_uev(nal);
  printf("height:%d\n", sps->pic_height_in_luma_samples);
  // sps->pic_height_in_luma_samples = read_uev(nal);
  // printf("height 1:%d\n", sps->pic_height_in_luma_samples);
  // sps->pic_height_in_luma_samples = read_uev(nal);
  // printf("height 2:%d\n", sps->pic_height_in_luma_samples);
}

// #define sps_payload 0x00 00 00 01 42 01 01 01 60 00 00 03 00 B0 00 00 03 00 00 03 00 5D A0 02 80 80 2D 16 36 AA 49 32
int main(int argc, char **argv)
{
//  uint8_t pPaylad[] = { 0x00, 0x00, 0x00, 0x01, 0x42, 0x01, 0x01, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00, 0xB0, 0x00, 0x00, 0x03,0x00,0x00,0x03, 0x00, 0x5D, 0xA0, 0x02, 0x80, 0x80, 0x2D, 0x16, 0x36, 0xAA, 0x49, 0x32}; //hikvision
     //    uint8_t pPaylad[] = {
    //     0x00, 0x00,0x00,0x01,0x42, 0x01 ,0x01 ,0x01 ,0x60 ,0x00 ,0x00 ,0x03 ,0x00 ,0xB0 ,0x00 ,0x00 ,0x03 ,0x00 ,0x00 ,0x03  ,0x00 ,0x7B ,0xA0 ,0x03 ,0xC0 ,0x80 ,0x10 ,0xE5 ,0x8D ,0xAE ,0x49 ,0x32 ,0xF1
    // }; //example 2
  // uint8_t pPaylad[] ={
  //   0x00,0x00,0x00,0x01,0x42,0x01,0x01,0x00,0x80,0x00,0x00,0x03,0x00,0x00,0x03,0x00,0x00,0x03,0x00,
  //   0x00,0x03,0x00,0x00,0xa0,0x02,0x80,0x80,0x2d,0x1f,0xe5,0x94,0x99,0x24,0x6d,0x86,
  //   0x16,0x22,0xaa,0x4c,0x4c,0x32,0xfb,0x3e,0xbc,0xdf,0x96,0x7d, 0x78,0x51,0x18,0x9c,
  //   0xbb,0x20}; //example 3
  // uint8_t pPaylad[] ={
  //   0x00, 0x00, 0x00, 0x01, 0x42, 0x01, 0x01, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00, 0xB0, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x99, 0xA0, 0x01, 0x40, 0x20, 0x05, 0xA1, 0x63, 0x6A, 0xA4, 0x93
  // };
  // uint8_t pPaylad[] ={
    // 0x00, 0x00, 0x00, 0x01, 0x42, 0x01, 0x01, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00, 0xB0, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x99, 0xA0, 0x01,0x00,0x20,0x06,0x01,0x63,0x6A,0xA4,0x03
  // };
    uint8_t pPaylad[] ={
        0x00, 0x00, 0x00, 0x01, 0x42, 0x01, 0x01, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00, 0xB0, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x99, 0xA0, 0x01, 0x00, 0x20,0x06,0x01,0x63,0x6A,0xA4,0x93
    };

  uint8_t *payload = (uint8_t*)malloc(128);
  // printf("size of pPaylad: %d\n", sizeof(pPaylad));
  memcpy(payload, pPaylad, sizeof(pPaylad));

  sps_t *sps = (sps_t*)malloc(sizeof(sps_t));
  memset(sps, 0, sizeof(sps_t));
  nalbuf_t *nal_data = (nalbuf_t*)malloc(sizeof(nalbuf_t));
  memset(nal_data, 0, sizeof(nalbuf_t));

  process_nalu(payload, 34, nal_data);
  profile_tier_level_data = (profile_tier_level_data_t*)malloc(sizeof(profile_tier_level_data_t));
  memset(profile_tier_level_data, 0, sizeof(profile_tier_level_data_t));
  // memset(nal_data, 0x00, sizeof(nalbuf_t));
  // nal_data->buf = payload;
  // nal_data->pos = -1;
  parse_sps(sps, nal_data);
}

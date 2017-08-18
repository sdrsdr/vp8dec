#include <stdio.h>
#include <stdlib.h>
#include <vpx/vpx_decoder.h>
#include <vpx/vp8dx.h>

vpx_codec_iface_t *vp8d;
vpx_codec_ctx_t ctx;
vpx_codec_dec_cfg_t cfg;

int main(int argc, char **argv)
{	
	if (argc!=2) {
		printf("We need a file with a keyframe pls\n");
		return -1;
	}
	FILE *f=fopen(argv[1],"rb");
	fseek(f,0, SEEK_END);
	unsigned sz=ftell(f);
	rewind(f);
	printf("%s has size of %u\n",argv[1],sz);
	void *frame=malloc(sz);
	if (fread(frame,1,sz,f)!=sz) {
		printf("failed to read the frame in whole!\n");
		return -1;
	} else {
		printf("frame is now inmem! :)\n");
	}
	fclose(f);
	vp8d=vpx_codec_vp8_dx();
	printf("HOLA %s !\n",vpx_codec_iface_name(vp8d));
	vpx_codec_stream_info_t si; si.w=0; si.h=0;
	vpx_codec_err_t  err=vpx_codec_peek_stream_info(vp8d,frame,sz,&si);
	printf("info ok? %u w:%u h:%u\n",(unsigned) err,si.w, si.h);
	
	
	//vpx_codec_dec_init(&ctx,vp8d,);
	return 0;
}

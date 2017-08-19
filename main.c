#include <stdio.h>
#include <stdlib.h>
#include <vpx/vpx_decoder.h>
#include <vpx/vp8dx.h>

vpx_codec_iface_t *vp8d;
vpx_codec_ctx_t ctx;
vpx_codec_dec_cfg_t cfg;


void gotframe (void *user_priv, const vpx_image_t *img){
	printf("GOT IMG!\n");
}

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
	vpx_codec_stream_info_t si; si.sz=sizeof(si); si.w=0; si.h=0;
	vpx_codec_err_t  err=vpx_codec_peek_stream_info(vp8d,frame,sz,&si);
	printf("info err=%u w=%u h=%u\n",(unsigned) err,si.w, si.h);
	
	vpx_codec_dec_cfg_t cfg; cfg.w=si.w; cfg.h=si.h; cfg.threads=1;
	err=vpx_codec_dec_init(&ctx,vp8d,&cfg,0);
	if (err!=VPX_CODEC_OK) {
		printf("failed to initialize codec context: %s :(\n",vpx_codec_err_to_string(err));
		return -1;
	}
// 	err=vpx_codec_register_put_frame_cb(&ctx,gotframe,NULL);
// 	if (err!=VPX_CODEC_OK) {
// 		printf("failed to regster codec frame cb: %s :(\n",vpx_codec_err_to_string(err));
// 		return -1;
// 	}
	err=vpx_codec_decode(&ctx,frame,sz,NULL,0);
	if (err!=VPX_CODEC_OK) {
		printf("failed to decode frame: %s :(\n",vpx_codec_err_to_string(err));
		return -1;
	}
	vpx_codec_iter_t it=NULL;
	vpx_image_t *img= vpx_codec_get_frame(&ctx,&it);
	while (img){ 
		printf("Got img to handle!\n");
		img= vpx_codec_get_frame(&ctx,&it);
	}
	vpx_codec_destroy(&ctx);
	
	return 0;
}

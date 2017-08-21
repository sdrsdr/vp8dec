#include <stdio.h>
#include <stdlib.h>
#include <vpx/vpx_decoder.h>
#include <vpx/vp8dx.h>
#include <turbojpeg.h>

vpx_codec_iface_t *vp8d;
vpx_codec_ctx_t ctx;
vpx_codec_dec_cfg_t cfg;

#define MAX(a,b) ((a)>(b)?(a):(b))
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
		printf("Got img to handle fmt:%u bd:%u w:%u/%u/%u h:%u/%u/%u chrshift:%ux%u clrrng:%u cs:%u\n",img->fmt,img->bit_depth,img->w,img->d_w,img->r_w,img->h,img->d_h,img->r_h,img->x_chroma_shift,img->y_chroma_shift,img->range,img->cs);
		unsigned fmt=img->fmt;
		if (fmt!=VPX_IMG_FMT_I420) {
			printf("Decoded image is not in I420 format?! :(\n");
			return -1;
		}
		unsigned char yuv_line[MAX(img->d_w*2+1,200)];
		snprintf(yuv_line,200,"%s.yuv",argv[1]);
		FILE *of=fopen(yuv_line,"wb");
		if (!of) {
			printf("Failed to open YUV422 output file :%s\n",yuv_line);
			return -1;
		}
		printf("YUV422 output to:%s\n",yuv_line);
		unsigned char *y=img->planes[VPX_PLANE_Y],*u=img->planes[VPX_PLANE_U],*v=img->planes[VPX_PLANE_V];
		for (int row=0; row<img->d_h; row++,y+=img->stride[VPX_PLANE_Y]) {
			for(int yx=0,uvy=0,py=0; yx<img->d_w; yx+=2, uvy++, py+=4){
				yuv_line[py+0]=u[uvy];
				yuv_line[py+1]=y[yx];
				yuv_line[py+2]=v[uvy];
				yuv_line[py+3]=y[yx+1];
			}
			fwrite(yuv_line,1,img->d_w*2,of);
			if (row & 1 ) {
				u+=img->stride[VPX_PLANE_U];
				v+=img->stride[VPX_PLANE_V];
			}
		}
		fclose(of);
		
		tjhandle tjh=tjInitCompress();
		unsigned char *jpegdata=NULL;
		unsigned long  jpegdata_len=0;
		if (tjCompressFromYUVPlanes(tjh,(const unsigned char **)(img->planes),img->d_w,img->stride,img->d_h,TJSAMP_420,&jpegdata,&jpegdata_len,85,0)==0) {
			snprintf(yuv_line,200,"%s.jpg",argv[1]);
			FILE *of=fopen(yuv_line,"wb");
			if (!of) {
				printf("Failed to open JPG output file :%s\n",yuv_line);
				return -1;
			}
			if (fwrite(jpegdata,1,jpegdata_len,of)!=jpegdata_len) {
				printf("Failed to write JPG data to output file :%s\n",yuv_line);
				return -1;
			}
			fclose(of);
			printf("libjpegturbo compress OK! %lu jpeg bytes written to %s\n",jpegdata_len,yuv_line);
		} else {
			printf("libjpegturbo compress failed :(\n");
		}
		if (jpegdata) tjFree(jpegdata);
		tjDestroy(tjh);
		img= vpx_codec_get_frame(&ctx,&it);
	}
	vpx_codec_destroy(&ctx);
	
	return 0;
}

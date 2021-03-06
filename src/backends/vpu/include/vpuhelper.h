//------------------------------------------------------------------------------
// File: vpuhelper.h
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------

#ifndef _VPU_HELPER_H_
#define _VPU_HELPER_H_

#include "vpuapi.h"

#include "vpuio.h"
#ifdef SUPPORT_FFMPEG_DEMUX
#if defined (__cplusplus)
extern "C" {
#endif
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#if defined (__cplusplus)
}
#endif
#endif
// Decoder Log file name to report information
#define FN_PIC_INFO "dec_pic_disp_info.log"
#define FN_SEQ_INFO "dec_seq_disp_info.log"
#define FN_PIC_TYPE "dec_pic_type.log"
#define FN_USER_DATA "dec_user_data.log"
#define FN_SEQ_USER_DATA "dec_seq_user_data.log"

#define	USER_DATA_INFO_OFFSET			(8*17)



typedef struct VpuReportContext_t VpuReportContext_t;
typedef struct VpuReportConfig_t VpuReportConfig_t;

struct VpuReportConfig_t {
	PhysicalAddress userDataBufAddr;
	int userDataEnable;
    int userDataReportMode; // (0 : interrupt mode, 1 interrupt disable mode)
    int userDataBufSize;

};

struct VpuReportContext_t {
    VpuReportConfig_t cfg;
	osal_file_t fpUserDataLogfile;
    osal_file_t fpSeqUserDataLogfile;    
    int decIndex;
};



typedef enum {                                   // start code
	NAL_TYPE_NON_IDR        = 0x01,
	NAL_TYPE_DP_A           = 0x02,
	NAL_TYPE_DP_B           = 0x03,
	NAL_TYPE_DP_C           = 0x04,
	NAL_TYPE_IDR            = 0x05,
	NAL_TYPE_SEI            = 0x06,
	NAL_TYPE_SEQ_PARA       = 0x07,
	NAL_TYPE_PIC_PARA       = 0x08,
	NAL_TYPE_AUD            = 0x09,
	NAL_TYPE_EO_SEQ         = 0x0a,
	NAL_TYPE_EO_STR         = 0x0b,
	NAL_TYPE_FILLER         = 0x0c,
	NAL_TYPE_SEQ_PARA_EXT   = 0x0d, 
} NAL_TYPE;


typedef struct {
	int* buffer;
	int  size;
	int  count;
	int  front;
	int  rear;
} frame_queue_item_t;


#if defined (__cplusplus)
extern "C"{
#endif 
	
	void CheckVersion(Uint32 core_idx);
	void PrintMemoryAccessViolationReason(Uint32 core_idx, DecOutputInfo *out);
	void PrintSEI(DecOutputInfo *out);
	int fourCCToCodStd(unsigned int  fourcc);
	int fourCCToMp4Class(unsigned int  fourcc);
	int codecIdToCodStd(int codec_id);
	int codecIdToFourcc(int codec_id);
	int codecIdToMp4Class(int codec_id);	
	

    int MaverickCache1Config(MaverickCacheConfig* pCache, int bypass, int mapType, int cbcrIntlv, int dual);

#ifdef SUPPORT_FFMPEG_DEMUX
	//make from ffmpeg
	int BuildSeqHeader(BYTE *pbHeader, const CodStd codStd, const AVStream *st);
	int BuildPicHeader(BYTE *pbHeader, const CodStd codStd, const AVStream *st, const AVPacket *pkt);
	int BuildPicData(BYTE *pbChunk, const CodStd codStd, const AVStream *st, const AVPacket *pkt);
#endif
	// Decoder Report Information
	void ConfigSeqReport(Uint32 core_idx, DecHandle handle, CodStd bitstreamFormat);
	void SaveSeqReport(Uint32 core_idx, DecHandle handle, DecInitialInfo *pSeqInfo, CodStd bitstreamFormat);
	void ConfigDecReport(Uint32 core_idx, DecHandle handle, CodStd bitstreamFormat);
	void SaveDecReport(Uint32 core_idx, DecHandle handle, DecOutputInfo *pDecInfo, CodStd bitstreamFormat, int mbNumX);
	void CheckUserDataInterrupt(Uint32 core_idx, DecHandle handle, int decodeIdx, CodStd bitstreamFormat, int int_reason);
    	
	
	// Bitstream Buffer Control
	RetCode WriteBsBufHelper(Uint32 core_idx, 
		DecHandle handle,
		BufInfo *pBufInfo,
	    vpu_buffer_t *pVbStream,
	    int defaultsize, int checkeos,
	    int *pstreameos,
	    int endian);
	int WriteBsBufFromBufHelper(Uint32 core_idx, 
		DecHandle handle,
		vpu_buffer_t *pVbStream,
		BYTE *pChunk, 
		int chunkSize,
		int endian);
	int GetBitstreamBufferRoom(DecHandle handle);
	int find_next_idr_nonidr_nal(BYTE *data, int len, int *sc_ptr, int *sc_num);
	int read_enc_ringbuffer_data(Uint32 core_idx, 
		EncHandle handle, 
		osal_file_t bsFp, 
		PhysicalAddress bitstreamBuffer,
		Uint32 bitstreamBufferSize, 
		int endian);
	int FillBsResetBufHelper(Uint32 core_idx,
		unsigned char *buf,
		PhysicalAddress paBsBufAddr,
	    int bsBufsize,
	    int endian);
    RetCode ReadBsRingBufHelper(Uint32 core_idx,
		EncHandle handle,
		osal_file_t bsFp,
		PhysicalAddress bitstreamBuffer,
		Uint32 bitstreamBufferSize,
		int defaultsize,
		int endian);
	int FillBsRingBufHelper(Uint32 core_idx,
		EncHandle handle,
		unsigned char *buf,
		PhysicalAddress paBsBufStart,
		PhysicalAddress paBsBufEnd,
	    int defaultsize,
	    int endian);
	// DPB Image Data Control
	int LoadYuvImageHelperFormat(Uint32 core_idx,
		osal_file_t yuvFp,
		Uint8 *pYuv,
		FrameBuffer	*fb,
		TiledMapConfig mapCfg,
		int picWidth,
		int picHeight,
		int stride,
		int interleave,
		int format,
		int endian);

    int SaveYuvImageHelperFormat(Uint32 core_idx,
        osal_file_t yuvFp,
        FrameBuffer *fbSrc,
		TiledMapConfig mapCfg,
        Uint8 *pYuv, 
        Rect rect,
        int interLeave,
        int format,
        int endian);


	// Bitstream Data Control
	int ReadBsResetBufHelper(Uint32 core_idx,
		osal_file_t streamFp,
		PhysicalAddress bitstream,
		int size,
		int endian);


	int IsSupportInterlaceMode(CodStd bitstreamFormat, DecInitialInfo *pSeqInfo);	
	
	void init_VSYNC_flag();
	void clear_VSYNC_flag();
	int check_VSYNC_flag();
	void set_VSYNC_flag();
	frame_queue_item_t* frame_queue_init(int count);
	void frame_queue_deinit(frame_queue_item_t* queue);
	int frame_queue_enqueue(frame_queue_item_t* queue, int data);
	int frame_queue_dequeue(frame_queue_item_t* queue, int* data);
	int frame_queue_dequeue_all(frame_queue_item_t* queue);
	int frame_queue_peekqueue(frame_queue_item_t* queue, int* data);
	int frame_queue_count(frame_queue_item_t* queue);
	int frame_queue_check_in_queue(frame_queue_item_t* queue, int val);
/* make container */
#define MAX_HEADER_CNT 3
#define CONTAINER_HEADER_SIZE (50*MAX_HEADER_CNT)
#if defined (__cplusplus)
}
#endif 

#endif //#ifndef _VPU_HELPER_H_

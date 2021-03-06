#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <memory.h>

#include "mxc_ipu.h"



static ipu_lib_handle_t* global_ipu_handle = NULL;
static int next_update_index = 0;
static int skip_next_frame = 0;


static void ipu_output_callback(void* output, int index)
{
	if(output) MEMCPY(output, global_ipu_handle->outbuf_start[index], global_ipu_handle->ofr_size);
}


int ipu_query_task(void)
{
	int i;
	ipu_lib_ctl_task_t task;

	for (i = 0; i< MAX_TASK_NUM; i++) {
		task.index = i;
		mxc_ipu_lib_task_control(IPU_CTL_TASK_QUERY, (void*)(&task), NULL);
		if (task.task_pid) {
			fprintf(stderr, "\ntask %d:\n", i);
			fprintf(stderr, "\tpid: %d\n", task.task_pid);
			fprintf(stderr, "\tmode:\n");
			if (task.task_mode & IC_ENC)
				fprintf(stderr, "\t\tIC_ENC\n");
			if (task.task_mode & IC_VF)
				fprintf(stderr, "\t\tIC_VF\n");
			if (task.task_mode & IC_PP)
				fprintf(stderr, "\t\tIC_PP\n");
			if (task.task_mode & ROT_ENC)
				fprintf(stderr, "\t\tROT_ENC\n");
			if (task.task_mode & ROT_VF)
				fprintf(stderr, "\t\tROT_VF\n");
			if (task.task_mode & ROT_PP)
				fprintf(stderr, "\t\tROT_PP\n");
			if (task.task_mode & VDI_IC_VF)
				fprintf(stderr, "\t\tVDI_IC_VF\n");
		}
	}

	return 0;
}

ipu_lib_handle_t* ipu_init(int in_w, int in_h, int in_fmt, int out_w, int out_h, int out_fmt, int show)
{
	ipu_lib_handle_t* ipu_handle = NULL;
	ipu_lib_input_param_t input;
	ipu_lib_output_param_t output;

	ipu_handle = calloc(1, sizeof(ipu_lib_handle_t));
	memset(&input, 0, sizeof(ipu_lib_input_param_t));
	memset(&output, 0, sizeof(ipu_lib_output_param_t));

	input.width = in_w;
	input.height = in_h;
	input.fmt = in_fmt;

	output.width = out_w;
	output.height = out_h;
	output.fmt = out_fmt;
	output.show_to_fb = show;

	output.output_win.pos.x = 0;
	output.output_win.pos.y = 0;
	output.output_win.win_w = out_w;
	output.output_win.win_h = out_h;

	mxc_ipu_lib_task_init(&input, NULL, &output, OP_NORMAL_MODE|TASK_VF_MODE, ipu_handle);

	return ipu_handle;
}

void ipu_uninit(ipu_lib_handle_t** ipu_handle)
{
	if(!*ipu_handle)
		return;

	mxc_ipu_lib_task_uninit(*ipu_handle);
	free(*ipu_handle);
	*ipu_handle = NULL;
}

void ipu_buffer_update(ipu_lib_handle_t* ipu_handle, const unsigned char* input_data, unsigned char* output_data)
{
	global_ipu_handle= ipu_handle;

	if(!skip_next_frame)
		MEMCPY(ipu_handle->inbuf_start[0], input_data, ipu_handle->ifr_size);

	next_update_index = mxc_ipu_lib_task_buf_update(ipu_handle, 0, 0, 0, ipu_output_callback, output_data);

	skip_next_frame = (next_update_index == -1);
}


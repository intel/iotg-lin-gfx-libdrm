/*
 * Copyright © 2014 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdint.h>

#include "i915_drm.h"
#include "i915_perfmon.h"
#include <xf86drm.h>

#include "intel_bufmgr.h"
#include "intel_bufmgr_priv.h"
#include "intel_perfmon.h"

#include "errno.h"

int
drm_intel_perfmon_enable_config(int fd, int enable)
{
	struct drm_i915_perfmon perfmon;
	int ret;

	perfmon.op = enable ?
			I915_PERFMON_ENABLE_CONFIG :
			I915_PERFMON_DISABLE_CONFIG;
	ret = drmIoctl(fd, DRM_IOCTL_I915_PERFMON, &perfmon);

	return ret;
}

int
drm_intel_perfmon_open(int fd)
{
	struct drm_i915_perfmon perfmon;
	int ret;

	perfmon.op = I915_PERFMON_OPEN;
	ret = drmIoctl(fd, DRM_IOCTL_I915_PERFMON, &perfmon);

	return ret;
}

int
drm_intel_perfmon_close(int fd)
{
	struct drm_i915_perfmon perfmon;
	int ret;

	perfmon.op = I915_PERFMON_CLOSE;
	ret = drmIoctl(fd, DRM_IOCTL_I915_PERFMON, &perfmon);

	return ret;
}

int
drm_intel_perfmon_set_config(
	int fd,
	enum DRM_I915_PERFMON_CONFIG_TARGET target,
	unsigned int pid,
	struct drm_i915_perfmon_config_entry *oa_entries,
	unsigned int num_oa_entries,
	unsigned int oa_id,
	struct drm_i915_perfmon_config_entry *gp_entries,
	unsigned int num_gp_entries,
	unsigned int gp_id)
{
	struct drm_i915_perfmon perfmon_ioctl;
	int ret;

	perfmon_ioctl.op = I915_PERFMON_SET_CONFIG;
	perfmon_ioctl.data.set_config.target = target;
	perfmon_ioctl.data.set_config.pid = pid;
	perfmon_ioctl.data.set_config.oa.entries = (uintptr_t)oa_entries;
	perfmon_ioctl.data.set_config.oa.size	= num_oa_entries;
	perfmon_ioctl.data.set_config.oa.id	= oa_id;
	perfmon_ioctl.data.set_config.gp.entries = (uintptr_t)gp_entries;
	perfmon_ioctl.data.set_config.gp.size	= num_gp_entries;
	perfmon_ioctl.data.set_config.gp.id 	= gp_id;

	ret = drmIoctl(fd, DRM_IOCTL_I915_PERFMON, &perfmon_ioctl);

	return ret;
}

int
drm_intel_perfmon_load_config(
	int fd,
	drm_intel_context *ctx,
	uint32_t *oa_cfg_id,
	uint32_t *gp_cfg_id)
{
	struct drm_i915_perfmon perfmon;
	int ret;

	perfmon.op = I915_PERFMON_LOAD_CONFIG;
	perfmon.data.load_config.ctx_id = ctx ? ctx->ctx_id : 0;
	perfmon.data.load_config.oa_id = *oa_cfg_id;
	perfmon.data.load_config.gp_id = *gp_cfg_id;
	ret = drmIoctl(fd, DRM_IOCTL_I915_PERFMON, &perfmon);

	*oa_cfg_id = perfmon.data.load_config.oa_id;
	*gp_cfg_id = perfmon.data.load_config.gp_id;

	return ret;
}


int
drm_intel_perfmon_get_hw_ctx_id(
	int fd,
	drm_intel_context *ctx,
	unsigned int *hw_ctx_id)
{
	struct drm_i915_perfmon perfmon;
	int ret;

	perfmon.op = I915_PERFMON_GET_HW_CTX_ID;
	perfmon.data.get_hw_ctx_id.ctx_id = ctx ? ctx->ctx_id : 0;
	ret = drmIoctl(fd, DRM_IOCTL_I915_PERFMON, &perfmon);

	*hw_ctx_id = perfmon.data.get_hw_ctx_id.hw_ctx_id;
	return ret;
}

int
drm_intel_perfmon_get_hw_ctx_ids(
	int fd,
	int pid,
	unsigned int *hw_ctx_ids,
	unsigned int *hw_ctx_ids_count)
{
	struct drm_i915_perfmon perfmon;
	int ret;

	perfmon.op = I915_PERFMON_GET_HW_CTX_IDS;
	perfmon.data.get_hw_ctx_ids.pid = pid;
	perfmon.data.get_hw_ctx_ids.count = *hw_ctx_ids_count;
	perfmon.data.get_hw_ctx_ids.ids = (uintptr_t)hw_ctx_ids;

	ret = drmIoctl(fd, DRM_IOCTL_I915_PERFMON, &perfmon);
	*hw_ctx_ids_count = perfmon.data.get_hw_ctx_ids.count;

	return ret;

}

int
drm_intel_perfmon_pin_oa_buffer(
	int fd,
	drm_intel_bo *bo,
	uint32_t alignment)
{
	struct drm_i915_perfmon perfmon;
	int ret;

	perfmon.op = I915_PERFMON_PIN_OA_BUFFER;
	perfmon.data.pin_oa_buffer.handle = bo->handle;
	perfmon.data.pin_oa_buffer.alignment = alignment;
	perfmon.data.pin_oa_buffer.offset = 0UL;

	ret = drmIoctl(fd, DRM_IOCTL_I915_PERFMON, &perfmon);

	if (ret != 0)
		return -errno;

	bo->offset64 = perfmon.data.pin_oa_buffer.offset;
	bo->offset = perfmon.data.pin_oa_buffer.offset;

	return ret;
}


int
drm_intel_perfmon_unpin_oa_buffer(
	int fd,
	drm_intel_bo *bo)
{
	struct drm_i915_perfmon perfmon;
	int ret;

	perfmon.op = I915_PERFMON_UNPIN_OA_BUFFER;
	perfmon.data.unpin_oa_buffer.handle = bo->handle;

	ret = drmIoctl(fd, DRM_IOCTL_I915_PERFMON, &perfmon);

	if (ret != 0)
		return -errno;

	return 0;
}

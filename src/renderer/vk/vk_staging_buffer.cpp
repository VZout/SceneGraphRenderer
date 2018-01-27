#include "vk_staging_buffer.h"

#include "../interface.h"
#include "vk_internal.h"
#include "vk_device.h"

namespace rlr {

void Create(StagingBuffer& buffer, Device& device, void* data, uint64_t size, unsigned char usage_flags, int properties) {
	SET_INTERNAL_DEVICE_PTR(buffer, device)

	buffer.m_size = size;

	vk::Device n_device = device.device;

	internal_vk::CreateBuffer(buffer.staging_buffer, buffer.staging_buffer_memory, device, size, BufferUsageFlag::COPY_SOURCE, (int)vk::MemoryPropertyFlagBits::eHostVisible | (int)vk::MemoryPropertyFlagBits::eHostCoherent);

	if (data != nullptr) {
		void* temp_data;
		n_device.mapMemory(buffer.staging_buffer_memory, 0, size, vk::MemoryMapFlagBits(0), &temp_data); //FIXME	wtf vulkan... memorymapflagbits is not implemented hence why I can't just type 0. Now I have to type vk::MemoryMapFlagBits(0)
		memcpy(temp_data, data, (size_t)size);
		n_device.unmapMemory(buffer.staging_buffer_memory);
	}

	internal_vk::CreateBuffer(buffer.buffer, buffer.buffer_memory, device, size, usage_flags, properties);
}

void Destroy(StagingBuffer& buffer) {
	vk::Device n_device = GET_NATIVE_INTERNAL_DEVICE(buffer);

	n_device.destroyBuffer(buffer.buffer);
	n_device.freeMemory(buffer.buffer_memory);

	n_device.destroyBuffer(buffer.staging_buffer);
	n_device.freeMemory(buffer.staging_buffer_memory);
}

void Update(StagingBuffer& buffer, Device& device, vk::DeviceSize size, void* data) {
	vk::Device n_device = device.device;

	void* temp_data;
	n_device.mapMemory(buffer.staging_buffer_memory, 0, size, vk::MemoryMapFlagBits(0), &temp_data); //FIXME	wtf vulkan... memorymapflagbits is not implemented hence why I can't just type 0. Now I have to type vk::MemoryMapFlagBits(0)
	memcpy(temp_data, data, (size_t)size);
	n_device.unmapMemory(buffer.staging_buffer_memory);
}

void StageBuffer(StagingBuffer& buffer, CommandList& cmd_list) {
	vk::CommandBuffer n_cmd_list = cmd_list.m_native[cmd_list.m_current_frame_idx];

	vk::BufferCopy copyRegion = {};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = buffer.m_size;

	n_cmd_list.copyBuffer(buffer.staging_buffer, buffer.buffer, 1, &copyRegion);
}

void FreeStagingBuffer(StagingBuffer& buffer) {
	if (buffer.staging_buffer) GET_NATIVE_INTERNAL_DEVICE(buffer).destroyBuffer(buffer.staging_buffer);
	if (buffer.staging_buffer_memory) GET_NATIVE_INTERNAL_DEVICE(buffer).freeMemory(buffer.staging_buffer_memory);
}

} /* rlr */

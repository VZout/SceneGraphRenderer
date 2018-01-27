#include "vk_texture.h"

#include "../interface.h"
#include "vk_internal.h"

#include <D3Dcompiler.h>
#include <wrl.h>
#include <wincodec.h>
#include <fstream>
#include <iostream>
#include <string>

namespace rlr {

	// TODO: Remaining formats
inline Format GetDXGIFormatFromWICFormat(WICPixelFormatGUID & wicFormatGUID) {
	if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFloat) return Format::R32G32B32A32_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAHalf) return Format::R16G16B16A16_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBA) return Format::R16G16B16A16_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA) return Format::R8G8B8A8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppBGRA) return Format::B8G8R8A8_UNORM;
	//else if (wicFormatGUID == GUID_WICPixelFormat32bppBGR) return Format::B8G8R8X8_UNORM;
	//else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102XR) return Format::R10G10B10_XR_BIAS_A2_UNORM;

	//else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102) return Format::R10G10B10A2_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppBGRA5551) return Format::B5G5R5A1_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppBGR565) return Format::B5G6R5_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFloat) return Format::R32_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppGrayHalf) return Format::R16_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppGray) return Format::R16_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat8bppGray) return Format::R8_UNORM;
	//else if (wicFormatGUID == GUID_WICPixelFormat8bppAlpha) return Format::A8_UNORM;

	else return Format::UNKNOWN;
}


inline WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID & wicFormatGUID) {
	if (wicFormatGUID == GUID_WICPixelFormatBlackWhite) return GUID_WICPixelFormat8bppGray;
	else if (wicFormatGUID == GUID_WICPixelFormat1bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat2bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat4bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat8bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat2bppGray) return GUID_WICPixelFormat8bppGray;
	else if (wicFormatGUID == GUID_WICPixelFormat4bppGray) return GUID_WICPixelFormat8bppGray;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppGrayFixedPoint) return GUID_WICPixelFormat16bppGrayHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFixedPoint) return GUID_WICPixelFormat32bppGrayFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppBGR555) return GUID_WICPixelFormat16bppBGRA5551;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppBGR101010) return GUID_WICPixelFormat32bppRGBA1010102;
	else if (wicFormatGUID == GUID_WICPixelFormat24bppBGR) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat24bppRGB) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppPBGRA) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppPRGBA) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppRGB) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppBGR) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppBGRA) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBA) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppPBGRA) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppBGRFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppBGRAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppPRGBAFloat) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFloat) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBE) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppCMYK) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppCMYK) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat40bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat80bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGB) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGB) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBAHalf) return GUID_WICPixelFormat64bppRGBAHalf;
#endif

	else return GUID_WICPixelFormatDontCare;
}

// TODO: Remaining formats
inline int GetDXGIFormatBitsPerPixel(Format& format) {
	if (format == Format::R32G32B32A32_FLOAT) return 128;
	else if (format == Format::R16G16B16A16_FLOAT) return 64;
	else if (format == Format::R16G16B16A16_UNORM) return 64;
	else if (format == Format::R8G8B8A8_UNORM) return 32;
	else if (format == Format::B8G8R8A8_UNORM) return 32;
	//else if (format == Format::B8G8R8X8_UNORM) return 32;
	//else if (format == Format::R10G10B10_XR_BIAS_A2_UNORM) return 32;

	//else if (format == Format::R10G10B10A2_UNORM) return 32;
	else if (format == Format::B5G5R5A1_UNORM) return 16;
	else if (format == Format::B5G6R5_UNORM) return 16;
	else if (format == Format::R32_FLOAT) return 32;
	else if (format == Format::R16_FLOAT) return 16;
	else if (format == Format::R16_UNORM) return 16;
	else if (format == Format::R8_UNORM) return 8;
	//else if (format == Format::A8_UNORM) return 8;

	return -1;
}

int LoadImageDataFromMemory(BYTE** data, uint8_t* buffer, unsigned int size, TextureDescription& resourceDescription, int &bytesPerRow) {
	HRESULT hr;

	// we only need one instance of the imaging factory to create decoders and frames
	static IWICImagingFactory *wicFactory;

	// reset decoder, frame and converter since these will be different for each image we load
	IWICBitmapDecoder *wicDecoder = NULL;
	IWICBitmapFrameDecode *wicFrame = NULL;
	IWICFormatConverter *wicConverter = NULL;
	IWICStream *pIWICStream = NULL;

	bool imageConverted = false;

	if (wicFactory == NULL)
	{
		// Initialize the COM library
		CoInitialize(NULL);

		// create the WIC factory
		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&wicFactory)
		);
		if (FAILED(hr)) return 0;
	}

	hr = wicFactory->CreateStream(&pIWICStream);

	// Initialize the stream with the memory pointer and size.
	if (SUCCEEDED(hr)) {
		hr = pIWICStream->InitializeFromMemory(
			reinterpret_cast<BYTE*>(buffer),
			size);
	}

	// load a decoder for the image
	hr = wicFactory->CreateDecoderFromStream(
		pIWICStream,                   // The stream to use to create the decoder
		NULL,                          // Do not prefer a particular vendor
		WICDecodeMetadataCacheOnLoad,  // Cache metadata when needed
		&wicDecoder                   // Pointer to the decoder
	);
	if (FAILED(hr))
		return 0;

	// get image from decoder (this will decode the "frame")
	hr = wicDecoder->GetFrame(0, &wicFrame);
	if (FAILED(hr)) return 0;

	// get wic pixel format of image
	WICPixelFormatGUID pixelFormat;
	hr = wicFrame->GetPixelFormat(&pixelFormat);
	if (FAILED(hr)) return 0;

	// get size of image
	UINT textureWidth, textureHeight;
	hr = wicFrame->GetSize(&textureWidth, &textureHeight);
	if (FAILED(hr)) return 0;

	// we are not handling sRGB types in this tutorial, so if you need that support, you'll have to figure
	// out how to implement the support yourself

	// convert wic pixel format to dxgi pixel format
	Format textureFormat = GetDXGIFormatFromWICFormat(pixelFormat);

	// if the format of the image is not a supported dxgi format, try to convert it
	if (textureFormat == Format::UNKNOWN)
	{
		// get a dxgi compatible wic format from the current image format
		WICPixelFormatGUID convertToPixelFormat = GetConvertToWICFormat(pixelFormat);

		// return if no dxgi compatible format was found
		if (convertToPixelFormat == GUID_WICPixelFormatDontCare) return 0;

		// set the dxgi format
		textureFormat = GetDXGIFormatFromWICFormat(convertToPixelFormat);

		// create the format converter
		hr = wicFactory->CreateFormatConverter(&wicConverter);
		if (FAILED(hr)) return 0;

		// make sure we can convert to the dxgi compatible format
		BOOL canConvert = FALSE;
		hr = wicConverter->CanConvert(pixelFormat, convertToPixelFormat, &canConvert);
		if (FAILED(hr) || !canConvert) return 0;

		// do the conversion (wicConverter will contain the converted image)
		hr = wicConverter->Initialize(wicFrame, convertToPixelFormat, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
		if (FAILED(hr)) return 0;

		// this is so we know to get the image data from the wicConverter (otherwise we will get from wicFrame)
		imageConverted = true;
	}

	int bitsPerPixel = GetDXGIFormatBitsPerPixel(textureFormat); // number of bits per pixel
	bytesPerRow = (textureWidth * bitsPerPixel) / 8; // number of bytes in each row of the image data
	int imageSize = bytesPerRow * textureHeight; // total image size in bytes

													// allocate enough memory for the raw image data, and set imageData to point to that memory
	*data = (BYTE*)malloc(imageSize);

	// copy (decoded) raw image data into the newly allocated memory (imageData)
	if (imageConverted)
	{
		// if image format needed to be converted, the wic converter will contain the converted image
		hr = wicConverter->CopyPixels(0, bytesPerRow, imageSize, *data);
		if (FAILED(hr)) return 0;
	}
	else
	{
		// no need to convert, just copy data from the wic frame
		hr = wicFrame->CopyPixels(0, bytesPerRow, imageSize, *data);
		if (FAILED(hr)) return 0;
	}

	resourceDescription.width = textureWidth;
	resourceDescription.height = textureHeight;
	resourceDescription.format = textureFormat;

															// return the size of the image. remember to delete the image once your done with it (in this tutorial once its uploaded to the gpu)
	return imageSize;
}

void Load(Texture& texture, std::string const & path) {
	std::ifstream fin(path.c_str(), std::ios::in | std::ios::binary);

	fin.seekg(0, fin.end);
	int size = fin.tellg();
	fin.seekg(0, fin.beg);

	texture.buffer = (char*)malloc(size);
	texture.size = size;
	fin.read(texture.buffer,  size);

	LoadImageDataFromMemory(&texture.data, (uint8_t*)texture.buffer, texture.size, texture.desc, texture.bytes_per_row);
}

void StageTexture(Texture& texture, Device& device, CommandList& cmd_list) {
	SET_INTERNAL_DEVICE_PTR(texture, device);

	// Create vk staging image and stage
	internal_vk::CreateImage(device, texture.desc.width, texture.desc.height, (vk::Format)texture.desc.format,
		vk::ImageTiling::eLinear,
		vk::ImageUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		texture.staging_img, texture.staging_img_mem);

	vk::ImageSubresource subresource = {};
	subresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	subresource.mipLevel = 0;
	subresource.arrayLayer = 0;

	vk::SubresourceLayout staging_img_layout;
	device.device.getImageSubresourceLayout(texture.staging_img, &subresource, &staging_img_layout);

	void* temp_data;
	device.device.mapMemory(texture.staging_img_mem, 0, texture.size, vk::MemoryMapFlagBits(0), &temp_data);

	if (staging_img_layout.rowPitch == texture.desc.width * 4) {
		memcpy(temp_data, texture.data, (size_t)texture.size);
	}
	else {
		uint8_t* dataBytes = reinterpret_cast<uint8_t*>(temp_data);

		for (int y = 0; y < texture.desc.height; y++) {
			memcpy(&dataBytes[y * staging_img_layout.rowPitch], &texture.data[y * texture.desc.width * 4], texture.desc.width * 4);
		}
	}

	device.device.unmapMemory(texture.staging_img_mem);

	// Create Resources
	ResourceCreateInfo create_info;
	create_info.tiling = ImageTiling::OPTIMAL;
	create_info.usage = ImageUsage::SAMPLED | ImageUsage::TRANSFER_DST;
	create_info.mem_properties = MemoryProperties::DEVICE_LOCAL;
	create_info.format = Format::R8G8B8A8_UNORM;
	create_info.width = texture.desc.width;
	create_info.height = texture.desc.height;
	create_info.aspect = ImageAspect::COLOR;
	Create(texture.resource, device, create_info);

	// Prep image
	vk::CommandBuffer n_cmd_list = cmd_list->m_native[cmd_list->m_current_frame_idx];
	internal_vk::TransitionImageLayout(*cmd_list, texture.staging_img, Format::R8G8B8A8_UNORM, ResourceState::GENERAL_READ, ResourceState::COPY_SOURCE);
	TransitionImageLayout(*cmd_list, texture.resource, Format::R8G8B8A8_UNORM, ResourceState::GENERAL_READ, ResourceState::COPY_DEST);
	internal_vk::CopyImage(n_cmd_list, texture.staging_img, texture.resource.image, texture.desc.width, texture.desc.height);
}

void Destroy(Texture& texture) {
	vk::Device n_device = GET_NATIVE_INTERNAL_DEVICE(texture);

	n_device.destroyImage(texture.staging_img);
	n_device.freeMemory(texture.staging_img_mem);

	Destroy(texture.resource);
}

} /* rlr */

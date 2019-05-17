#include "storage_manager.h"
#include <localcache_session.h>
#include <localcache_commands.h>
#include <sirius_locks.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

sirius::library::cache::local::storage::manager::manager(void)
	: _context(NULL)
	, _cache_limit(0)
	, _cache_full(FALSE)
{
	::InitializeSRWLock(&_clock);
	//::InitializeSRWLock(&_slock);
	::InitializeSRWLock(&_ulock);
}

sirius::library::cache::local::storage::manager::~manager(void)
{
}

int32_t sirius::library::cache::local::storage::manager::initialize(sirius::library::cache::local::server::context_t * context)
{
	_context = context;
	_cache_limit = int64_t(_context->cache_size_mib) * int64_t(1024) * int64_t(1024);

	/*
	char debug[MAX_PATH] = { 0 };
	_snprintf_s(debug, MAX_PATH, "cache_size_mib is %d\n", _context->cache_size_mib);
	::OutputDebugStringA(debug);
	_snprintf_s(debug, MAX_PATH, "cache_limit is %lld bytes\n", _cache_limit);
	::OutputDebugStringA(debug);
	*/

	BOOL available = TRUE;
	char fullpath[MAX_PATH] = { 0 };
	char datapath[MAX_PATH] = { 0 };
	char indexpath[MAX_PATH] = { 0 };
	if (PathIsRelativeA(_context->path))
	{
		char module_path[MAX_PATH] = { 0 };
		char * module_name = module_path;
		module_name += GetModuleFileNameA(NULL, module_name, (sizeof(module_path) / sizeof(*module_path)) - (module_name - module_path));
		if (module_name != module_path)
		{
			CHAR * slash = strrchr(module_path, '\\');
			if (slash != NULL)
			{
				module_name = slash + 1;
				_strset_s(module_name, strlen(module_name) + 1, 0);
			}
			else
			{
				_strset_s(module_path, strlen(module_path) + 1, 0);
			}
		}
		_snprintf_s(fullpath, MAX_PATH, "%s%s", module_path, _context->path);

		if (_access(fullpath, 0) != 0)
		{
			available = ::CreateDirectoryA(fullpath, NULL);
		}
		
		if (available)
		{
			_snprintf_s(indexpath, MAX_PATH, "%s", fullpath);
			_snprintf_s(datapath, MAX_PATH, "%s\\data", fullpath);
			if (_access(datapath, 0) != 0)
			{
				available = ::CreateDirectoryA(datapath, NULL);
			}
		}
	}
	else
	{
		_snprintf_s(fullpath, MAX_PATH, "%s", _context->path);
		if (_access(fullpath, 0) != 0)
		{
			available = ::CreateDirectoryA(fullpath, NULL);
		}

		if (available)
		{
			_snprintf_s(indexpath, MAX_PATH, "%s", fullpath);
			_snprintf_s(datapath, MAX_PATH, "%s\\data", fullpath);
			if (_access(datapath, 0) != 0)
			{
				available = ::CreateDirectoryA(datapath, NULL);
			}
		}
	}

	if (available)
	{
		strncpy_s(_index_path, indexpath, MAX_PATH);
		strncpy_s(_cache_path, datapath, MAX_PATH);

		BOOL created = FALSE;
		char indexfile[MAX_PATH] = { 0 };
		_snprintf_s(indexfile, MAX_PATH, "%s\\index.dat", _index_path);
		HANDLE index = ::CreateFileA(indexfile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (index == INVALID_HANDLE_VALUE)
		{
			index = ::CreateFileA(indexfile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			created = TRUE;
		}

		BOOL success = FALSE;
		int64_t offset = 0;
		if (index != INVALID_HANDLE_VALUE && !created)
		{
			//deserialize index information from disk
			std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t> cindex = NULL;
			//cindex size is 160(except lock)
			while (TRUE)
			{
				cindex = std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t>(new sirius::library::cache::local::storage::manager::cached_index_t);
				success = read(index, offset, cindex->hash, sizeof(cindex->hash));
				if (!success)
					break;
				offset += int64_t(sizeof(cindex->hash));

				success = read(index, offset, cindex->hitcount);
				if (!success)
					break;
				offset += int64_t(sizeof(cindex->hitcount));

				success = read(index, offset, cindex->size);
				if (!success)
					break;
				offset += int64_t(sizeof(cindex->size));

				success = read(index, offset, cindex->width);
				if (!success)
					break;
				offset += int64_t(sizeof(cindex->width));

				success = read(index, offset, cindex->height);
				if (!success)
					break;
				offset += int64_t(sizeof(cindex->height));

				success = read(index, offset, cindex->upload_needed);
				if (!success)
					break;
				offset += int64_t(sizeof(cindex->upload_needed));

				success = read(index, offset, cindex->ftell_needed);
				if (!success)
					break;
				offset += int64_t(sizeof(cindex->ftell_needed));

				success = read(index, offset, cindex->serialized);
				if (!success)
					break;
				offset += int64_t(sizeof(cindex->serialized));

				_cache.insert(std::make_pair(cindex->hash, cindex));
			}
		}

		if (index != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(index);
			index = INVALID_HANDLE_VALUE;
		}

		char debug[MAX_PATH] = { 0 };
		_snprintf_s(debug, MAX_PATH, "deserialized cache index size is [%d]\n", _cache.size());
		::OutputDebugStringA(debug);

		return sirius::library::cache::local::storage::manager::err_code_t::success;
	}
	else
	{
		return sirius::library::cache::local::storage::manager::err_code_t::fail;
	}
}

int32_t sirius::library::cache::local::storage::manager::release(void)
{
	BOOL success = FALSE;
	std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t> cindex = NULL;
	std::map<std::string, std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t>>::iterator iter;


	char indexfile[MAX_PATH] = { 0 };
	_snprintf_s(indexfile, MAX_PATH, "%s\\index.dat", _index_path);
	if (::PathFileExistsA(indexfile))
	{
		::DeleteFileA(indexfile);
	}

	HANDLE index = ::CreateFileA(indexfile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (index != INVALID_HANDLE_VALUE)
	{
		int64_t offset = 0;
		int64_t cache_size = 0;
		for (iter = _cache.begin(); iter != _cache.end(); iter++)
		{
			cindex = iter->second;
			success = write(index, offset, cindex->hash, sizeof(cindex->hash));
			if (!success)
				break;
			offset += int64_t(sizeof(cindex->hash));

			success = write(index, offset, cindex->hitcount);
			if (!success)
				break;
			offset += int64_t(sizeof(cindex->hitcount));

			success = write(index, offset, cindex->size);
			if (!success)
				break;
			offset += int64_t(sizeof(cindex->size));

			success = write(index, offset, cindex->width);
			if (!success)
				break;
			offset += int64_t(sizeof(cindex->width));

			success = write(index, offset, cindex->height);
			if (!success)
				break;
			offset += int64_t(sizeof(cindex->height));

			success = write(index, offset, cindex->upload_needed);
			if (!success)
				break;
			offset += int64_t(sizeof(cindex->upload_needed));

			success = write(index, offset, cindex->ftell_needed);
			if (!success)
				break;
			offset += int64_t(sizeof(cindex->ftell_needed));

			success = write(index, offset, cindex->serialized);
			if (!success)
				break;
			offset += int64_t(sizeof(cindex->serialized));

			cache_size++;
		}
		::CloseHandle(index);
		index = INVALID_HANDLE_VALUE;

		char debug[MAX_PATH] = { 0 };
		_snprintf_s(debug, MAX_PATH, "serialized cache index size is [%lld]\n", cache_size);
		::OutputDebugStringA(debug);
	}

	_context = NULL;
	return sirius::library::cache::local::storage::manager::err_code_t::success;
}

int32_t sirius::library::cache::local::storage::manager::start(void)
{
	unsigned int thread_id;
	_run = TRUE;
	_thread = (HANDLE)_beginthreadex(NULL, 0, process_cb, this, 0, &thread_id);
	if (_thread == NULL || _thread == INVALID_HANDLE_VALUE)
		return sirius::library::cache::local::storage::manager::err_code_t::fail;
	return sirius::library::cache::local::storage::manager::err_code_t::success;
}

int32_t sirius::library::cache::local::storage::manager::stop(void)
{
	if (_thread != NULL && _thread != INVALID_HANDLE_VALUE)
	{
		_run = FALSE;
		if (::WaitForSingleObject(_thread, INFINITE) == WAIT_OBJECT_0)
			::CloseHandle(_thread);
		_thread = INVALID_HANDLE_VALUE;
	}
	return sirius::library::cache::local::storage::manager::err_code_t::success;
}

int32_t sirius::library::cache::local::storage::manager::begin_uploading(const char * sessionid, const char * hash, int32_t fsize, int32_t width, int32_t height, const char * packet, int32_t size)
{
	//::OutputDebugStringA("begin_uploading\n");
	int32_t status = sirius::library::cache::local::storage::manager::err_code_t::fail;
	do
	{
		if (!_context)
		{
			status = sirius::library::cache::local::storage::manager::err_code_t::storage_manager_not_initialized;
			break;
		}

		std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t> cindex = NULL;
		std::map<std::string, std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t>>::iterator citer;
		{
			sirius::shared_scopedlock lock(&_clock);
			citer = _cache.find(hash);
			if (citer != _cache.end())
				cindex = citer->second;
		}
		if (!cindex)
		{
			/*
			char debug[MAX_PATH] = { 0 };
			_snprintf_s(debug, MAX_PATH, "cached_image_not_exist : [%s]\n", hash);
			::OutputDebugStringA(debug);
			*/
			status = sirius::library::cache::local::storage::manager::err_code_t::cached_image_not_exist;
			break;
		}
		else
		{
			sirius::shared_scopedlock lock(&cindex->lock);
			if (cindex->serialized)
			{
				//::OutputDebugStringA("uploading_write_already_exist\n");
				status = sirius::library::cache::local::storage::manager::err_code_t::uploading_write_already_exist;
				break;
			}
		}

		std::shared_ptr<sirius::library::cache::local::storage::manager::uploading_state_t> us = NULL;
		{
			sirius::exclusive_scopedlock lock(&_ulock);
			std::map<std::string, std::shared_ptr<sirius::library::cache::local::storage::manager::uploading_state_t>>::iterator iter = _uploading.find(sessionid);
			if (iter != _uploading.end())
			{
				us = iter->second;
			}
			else
			{
				us = std::shared_ptr<sirius::library::cache::local::storage::manager::uploading_state_t>(new sirius::library::cache::local::storage::manager::uploading_state_t);
				if (!us)
				{
					status = sirius::library::cache::local::storage::manager::err_code_t::uploading_memalloc_fail;
					break;
				}
				memset(us->hash, 0x00, sizeof(us->hash));
				strncpy_s(us->hash, hash, sizeof(us->hash));
				us->size = fsize;
				us->width = width;
				us->height = height;
				us->timestamp = 0;
				us->file = create_cache_file(width, height, us->hash);

				_uploading.insert(std::make_pair(sessionid, us));
			}
		}
		
		if (size > 0)
		{
			BOOL writingfail = FALSE;
			{
				sirius::shared_scopedlock lock(&us->lock);
				writingfail = !do_shared_writing(us->file, 0, packet, size);
			}

			if (writingfail)
			{
				sirius::exclusive_scopedlock lock(&_ulock);
				std::map<std::string, std::shared_ptr<sirius::library::cache::local::storage::manager::uploading_state_t>>::iterator iter = _uploading.find(sessionid);
				if (iter != _uploading.end())
					_uploading.erase(iter);
				status = sirius::library::cache::local::storage::manager::err_code_t::uploading_segment_write_fail;
				break;
			}
			else
			{
				sirius::exclusive_scopedlock lock(&us->lock);
				us->nprocessed += size;
			}
		}
		
		status = sirius::library::cache::local::storage::manager::err_code_t::success;
	} while (0);

	return status;
}

int32_t sirius::library::cache::local::storage::manager::processing_uploading(const char * sessionid, const char * hash, int32_t fsize, int32_t width, int32_t height, int32_t offset, const char * packet, int32_t size)
{
	//::OutputDebugStringA("processing_uploading\n");
	int32_t status = sirius::library::cache::local::storage::manager::err_code_t::fail;
	do
	{
		if (!_context)
		{
			status = sirius::library::cache::local::storage::manager::err_code_t::storage_manager_not_initialized;
			break;
		}

		std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t> cindex = NULL;
		std::map<std::string, std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t>>::iterator citer;
		{
			sirius::shared_scopedlock lock(&_clock);
			citer = _cache.find(hash);
			if (citer != _cache.end())
				cindex = citer->second;
		}
		if (!cindex)
		{
			/*
			char debug[MAX_PATH] = { 0 };
			_snprintf_s(debug, MAX_PATH, "cached_image_not_exist : [%s]\n", hash);
			::OutputDebugStringA(debug);
			*/
			status = sirius::library::cache::local::storage::manager::err_code_t::cached_image_not_exist;
			break;
		}
		else
		{
			sirius::shared_scopedlock lock(&cindex->lock);
			if (cindex->serialized)
			{
				//::OutputDebugStringA("uploading_write_already_exist\n");
				status = sirius::library::cache::local::storage::manager::err_code_t::uploading_write_already_exist;
				break;
			}
		}

		std::shared_ptr<sirius::library::cache::local::storage::manager::uploading_state_t> us = NULL;
		{
			sirius::exclusive_scopedlock lock(&_ulock);
			std::map<std::string, std::shared_ptr<sirius::library::cache::local::storage::manager::uploading_state_t>>::iterator iter = _uploading.find(sessionid);
			if (iter != _uploading.end())
			{
				us = iter->second;
			}
			else
			{
				us = std::shared_ptr<sirius::library::cache::local::storage::manager::uploading_state_t>(new sirius::library::cache::local::storage::manager::uploading_state_t);
				if (!us)
				{
					status = sirius::library::cache::local::storage::manager::err_code_t::uploading_memalloc_fail;
					break;
				}

				memset(us->hash, 0x00, sizeof(us->hash));
				strncpy_s(us->hash, hash, sizeof(us->hash));
				us->size = fsize;
				us->width = width;
				us->height = height;
				us->timestamp = 0;
				us->file = create_cache_file(width, height, us->hash);

				_uploading.insert(std::make_pair(sessionid, us));
				break;
			}
		}

		BOOL writingfail = FALSE;
		{
			sirius::shared_scopedlock lock(&us->lock);
			writingfail = !do_shared_writing(us->file, offset, packet, size);
		}

		if (writingfail)
		{
			sirius::exclusive_scopedlock lock(&_ulock);
			std::map<std::string, std::shared_ptr<sirius::library::cache::local::storage::manager::uploading_state_t>>::iterator iter = _uploading.find(sessionid);
			if (iter != _uploading.end())
				_uploading.erase(iter);
			status = sirius::library::cache::local::storage::manager::err_code_t::uploading_segment_write_fail;
			break;
		}
		else
		{
			sirius::exclusive_scopedlock lock(&us->lock);
			us->nprocessed += size;
		}

		status = sirius::library::cache::local::storage::manager::err_code_t::success;
	} while (0);

	return status;
}

int32_t sirius::library::cache::local::storage::manager::end_uploading(const char * sessionid, const char * hash, int32_t fsize, int32_t width, int32_t height, int32_t offset, const char * packet, int32_t size)
{
	//::OutputDebugStringA("end_uploading\n");
	int32_t status = sirius::library::cache::local::storage::manager::err_code_t::fail;
	std::shared_ptr<sirius::library::cache::local::storage::manager::uploading_state_t> us = NULL;
	do
	{
		if (!_context)
		{
			status = sirius::library::cache::local::storage::manager::err_code_t::storage_manager_not_initialized;
			break;
		}

		std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t> cindex = NULL;
		std::map<std::string, std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t>>::iterator citer;
		{
			sirius::shared_scopedlock lock(&_clock);
			citer = _cache.find(hash);
			if (citer != _cache.end())
				cindex = citer->second;
		}
		if (!cindex)
		{
			/*
			char debug[MAX_PATH] = { 0 };
			_snprintf_s(debug, MAX_PATH, "cached_image_not_exist : [%s]\n", hash);
			::OutputDebugStringA(debug);
			*/
			status = sirius::library::cache::local::storage::manager::err_code_t::cached_image_not_exist;
			break;
		}
		else
		{
			sirius::shared_scopedlock lock(&cindex->lock);
			if (cindex->serialized)
			{
				//::OutputDebugStringA("uploading_write_already_exist\n");
				status = sirius::library::cache::local::storage::manager::err_code_t::uploading_write_already_exist;
				break;
			}
		}

		{
			sirius::exclusive_scopedlock lock(&_ulock);
			std::map<std::string, std::shared_ptr<sirius::library::cache::local::storage::manager::uploading_state_t>>::iterator iter = _uploading.find(sessionid);
			if (iter != _uploading.end())
			{
				us = iter->second;
			}
			else
			{
				us = std::shared_ptr<sirius::library::cache::local::storage::manager::uploading_state_t>(new sirius::library::cache::local::storage::manager::uploading_state_t);
				if (!us)
				{
					status = sirius::library::cache::local::storage::manager::err_code_t::uploading_memalloc_fail;
					break;
				}

				memset(us->hash, 0x00, sizeof(us->hash));
				strncpy_s(us->hash, hash, sizeof(us->hash));
				us->size = fsize;
				us->width = width;
				us->height = height;
				us->timestamp = 0;
				us->file = create_cache_file(width, height, us->hash);

				_uploading.insert(std::make_pair(sessionid, us));
				break;
			}
		}

		if (size > 0)
		{
			BOOL writingfail = FALSE;
			{
				sirius::shared_scopedlock lock(&us->lock);
				writingfail = !do_shared_writing(us->file, offset, packet, size);
			}

			if (writingfail)
			{
				sirius::exclusive_scopedlock lock(&_ulock);
				std::map<std::string, std::shared_ptr<sirius::library::cache::local::storage::manager::uploading_state_t>>::iterator iter = _uploading.find(sessionid);
				if (iter != _uploading.end())
					_uploading.erase(iter);
				status = sirius::library::cache::local::storage::manager::err_code_t::uploading_segment_write_fail;
				break;
			}
			else
			{
				sirius::exclusive_scopedlock lock(&us->lock);
				us->nprocessed += size;
			}
		}


		BOOL processed = FALSE;
		for (int32_t index = 0; index < 200; index++)
		{
			{
				sirius::shared_scopedlock lock(&us->lock);
				if (us->size == us->nprocessed)
				{
					processed = TRUE;
					break;
				}
			}
			::Sleep(1);
		}

		if (processed)
		{
			status = create_serialized_cached_index(us);
		}

		{
			sirius::exclusive_scopedlock lock(&_ulock);
			std::map<std::string, std::shared_ptr<sirius::library::cache::local::storage::manager::uploading_state_t>>::iterator iter = _uploading.find(sessionid);
			if (iter != _uploading.end())
				_uploading.erase(iter);

			::CloseHandle(us->file);
			us->file = INVALID_HANDLE_VALUE;
		}

		if (!processed || (status != sirius::library::cache::local::storage::manager::err_code_t::success))
		{
			char filename[MAX_PATH] = { 0 };
			_snprintf_s(filename, MAX_PATH, "%s\\%dx%d\\%s.png", _cache_path, us->width, us->height, us->hash);
			if (PathFileExistsA(filename))
			{
				::DeleteFileA(filename);
				::OutputDebugStringA("DeleteFileA\n");
			}
		}
	} while (0);

	return status;
}

void sirius::library::cache::local::storage::manager::on_download(const char * sessionid, const char * hash, std::shared_ptr<sirius::library::cache::local::session> session)
{
	do
	{
		if (!_context)
		{
			cmd_download_end_res_t res;
			strncpy_s(res.sessionid, sessionid, sizeof(res.sessionid));
			res.code = sirius::library::cache::local::storage::manager::err_code_t::storage_manager_not_initialized;
			session->send(CMD_DOWNLOAD_END_RES, reinterpret_cast<const char*>(&res), sizeof(res));
			break;
		}

		std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t> cindex = NULL;
		{
			sirius::shared_scopedlock lock(&_clock);
			std::map<std::string, std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t>>::iterator citer = _cache.find(hash);
			if (citer != _cache.end())
				cindex = citer->second;
		}

		if (!cindex)
		{
			cindex = std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t>(new sirius::library::cache::local::storage::manager::cached_index_t);
			strncpy_s(cindex->hash, hash, sizeof(cindex->hash));
			cindex->hitcount = 1;
			cindex->size = MAXIMUM_IMAGE_SIZE;
			if (_cache_full)
			{
				cindex->ftell_needed = TRUE;
				cindex->upload_needed = FALSE;
			}
			else
			{
				cindex->ftell_needed = FALSE;
				cindex->upload_needed = TRUE;
			}
			cindex->serialized = FALSE;

			{
				sirius::exclusive_scopedlock lock(&_clock);
				_cache.insert(std::make_pair(cindex->hash, cindex));
			}
		}
		else
		{
#if defined(WIN64)
			::InterlockedAdd64(&cindex->hitcount, 1);
#else
			::InterlockedAdd(&cindex->hitcount, 1);
#endif
		}

		{
			sirius::shared_scopedlock lock(&cindex->lock);
			if (cindex->ftell_needed)
			{
				sirius::library::cache::local::cmd_download_end_res_t res;
				strncpy_s(res.sessionid, sessionid, sizeof(res.sessionid));
				res.code = sirius::library::cache::local::storage::manager::err_code_t::cached_image_need_fsize;
				session->send(CMD_DOWNLOAD_END_RES, reinterpret_cast<const char*>(&res), sizeof(res));
				//::OutputDebugStringA("CMD_DOWNLOAD_END_RES : cached_image_need_fsize\n");
				break;
			}

			if (cindex->upload_needed)
			{
				sirius::library::cache::local::cmd_download_end_res_t res;
				strncpy_s(res.sessionid, sessionid, sizeof(res.sessionid));
				res.code = sirius::library::cache::local::storage::manager::err_code_t::cached_image_need_upload;
				session->send(CMD_DOWNLOAD_END_RES, reinterpret_cast<const char*>(&res), sizeof(res));
				//::OutputDebugStringA("CMD_DOWNLOAD_END_RES : cached_image_need_upload\n");
				break;
			}

			if (!cindex->serialized)
			{
				sirius::library::cache::local::cmd_download_end_res_t res;
				strncpy_s(res.sessionid, sessionid, sizeof(res.sessionid));
				res.code = sirius::library::cache::local::storage::manager::err_code_t::cached_image_low_hitcount;
				session->send(CMD_DOWNLOAD_END_RES, reinterpret_cast<const char*>(&res), sizeof(res));
				//::OutputDebugStringA("CMD_DOWNLOAD_END_RES : cached_image_low_hitcount\n");
				break;
			}
		}

		int32_t nsndbuff = session->send_buffer_size();
		char * sndbuff = static_cast<char*>(malloc(nsndbuff));
		if (sndbuff)
		{
			char filename[MAX_PATH] = { 0 };
			{
				sirius::shared_scopedlock lock(&cindex->lock);
				_snprintf_s(filename, MAX_PATH, "%s\\%dx%d\\%s.png", _cache_path, cindex->width, cindex->height, cindex->hash);
			}

			HANDLE hashed = ::CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hashed == INVALID_HANDLE_VALUE)
			{
				sirius::library::cache::local::cmd_download_end_res_t res;
				strncpy_s(res.sessionid, sessionid, sizeof(res.sessionid));
				res.code = sirius::library::cache::local::storage::manager::err_code_t::cached_image_read_fail;
				session->send(CMD_DOWNLOAD_END_RES, reinterpret_cast<const char*>(&res), sizeof(res));
			}
			else
			{
				DWORD ntarget = ::GetFileSize(hashed, NULL);
				int32_t offset = 0;
				for (int32_t index = 0; TRUE; index++)
				{
					DWORD bytes2read = nsndbuff;
					DWORD bytesread = 0;
					BOOL bio = FALSE;

					if (index == 0)
						bytesread += sizeof(sirius::library::cache::local::cmd_download_begin_res_t);
					else
						bytesread += sizeof(sirius::library::cache::local::cmd_download_processing_res_t);

					BOOL eof = FALSE;
					do
					{
						DWORD nbread = 0;
						bio = ::ReadFile(hashed, sndbuff + bytesread, bytes2read - bytesread, &nbread, NULL);
						if (!bio)
							break;
						if (nbread == 0)
						{
							eof = TRUE;
							break;
						}
						bytesread += nbread;
						if (bytes2read == bytesread)
							break;
					} while (1);

					if (!bio)
					{
						sirius::library::cache::local::cmd_download_end_res_t res;
						strncpy_s(res.sessionid, sessionid, sizeof(res.sessionid));
						res.code = sirius::library::cache::local::storage::manager::err_code_t::cached_image_read_fail;
						session->send(CMD_DOWNLOAD_END_RES, reinterpret_cast<const char*>(&res), sizeof(res));

						char debug[MAX_PATH] = { 0 };
						_snprintf_s(debug, MAX_PATH, "on_download fail : error code is [%d]\n", ::GetLastError());
						::OutputDebugStringA(debug);

						break;
					}

					if (index == 0) //first packet
					{
						if (!eof)
						{
							sirius::library::cache::local::cmd_download_begin_res_t begin;
							strncpy_s(begin.sessionid, sessionid, sizeof(begin.sessionid));
							strncpy_s(begin.hash, hash, sizeof(begin.hash));
							begin.size = ntarget;
							memmove(sndbuff, &begin, sizeof(begin));
							if (session)
								session->send(CMD_DOWNLOAD_BEGIN_RES, const_cast<const char*>(sndbuff), nsndbuff);

							offset += (nsndbuff - sizeof(begin));
							::OutputDebugStringA("CMD_DOWNLOAD_BEGIN_RES\n");
						}
						else
						{
							sirius::library::cache::local::cmd_download_begin_res_t begin;
							strncpy_s(begin.sessionid, sessionid, sizeof(begin.sessionid));
							strncpy_s(begin.hash, hash, sizeof(begin.hash));
							begin.size = ntarget;
							memmove(sndbuff, &begin, sizeof(begin));
							if (session)
							{
								session->send(CMD_DOWNLOAD_BEGIN_RES, const_cast<const char*>(sndbuff), bytesread);
								//char debug[MAX_PATH] = { 0 };
								//_snprintf_s(debug, MAX_PATH, "download begin : header size[%d], bytesread[%d], payload[%d]\n", sizeof(begin), bytesread, bytesread - sizeof(begin));
								//::OutputDebugStringA(debug);
							}

							offset += (bytesread - sizeof(begin));
							//::OutputDebugStringA("CMD_DOWNLOAD_BEGIN_RES #2\n");


							sirius::library::cache::local::cmd_download_end_res_t end;
							strncpy_s(end.sessionid, sessionid, sizeof(end.sessionid));
							end.size = ntarget;
							end.offset = offset;
							end.code = sirius::library::cache::local::storage::manager::err_code_t::success;
							memmove(sndbuff, &end, sizeof(end));
							if (session)
							{
								session->send(CMD_DOWNLOAD_END_RES, const_cast<const char*>(sndbuff), sizeof(end));
								//char debug[MAX_PATH] = { 0 };
								//_snprintf_s(debug, MAX_PATH, "download end : header size[%d], payload is [%d]\n", sizeof(end), 0);
								//::OutputDebugStringA(debug);
							}

							//::OutputDebugStringA("CMD_DOWNLOAD_END_RES #2\n");
							break;
						}
					}
					else
					{
						if (!eof)
						{
							sirius::library::cache::local::cmd_download_processing_res_t proc;
							strncpy_s(proc.sessionid, sessionid, sizeof(proc.sessionid));
							proc.size = ntarget;
							proc.offset = offset;
							proc.reserved = 0;
							memmove(sndbuff, &proc, sizeof(proc));
							if (session)
								session->send(CMD_DOWNLOAD_PROCESSING_RES, const_cast<const char*>(sndbuff), nsndbuff);

							offset += (nsndbuff - sizeof(proc));
							//::OutputDebugStringA("CMD_DOWNLOAD_PROCESSING_RES #3\n");
						}
						else
						{
							sirius::library::cache::local::cmd_download_processing_res_t proc;
							strncpy_s(proc.sessionid, sessionid, sizeof(proc.sessionid));
							proc.size = ntarget;
							proc.offset = offset;
							proc.reserved = 0;
							memmove(sndbuff, &proc, sizeof(proc));
							if (session)
							{
								session->send(CMD_DOWNLOAD_PROCESSING_RES, const_cast<const char*>(sndbuff), bytesread);
								::OutputDebugStringA("CMD_DOWNLOAD_PROCESSING_RES\n");
							}
							offset += (bytesread - sizeof(proc));

							sirius::library::cache::local::cmd_download_end_res_t end;
							strncpy_s(end.sessionid, sessionid, sizeof(end.sessionid));
							end.size = ntarget;
							end.offset = offset;
							end.code = sirius::library::cache::local::storage::manager::err_code_t::success;
							memmove(sndbuff, &end, sizeof(end));
							if (session)
							{
								session->send(CMD_DOWNLOAD_END_RES, const_cast<const char*>(sndbuff), sizeof(end));
								::OutputDebugStringA("CMD_DOWNLOAD_END_RES\n");
							}
							break;
						}
					}
				}
			}
			free(sndbuff);
			sndbuff = NULL;
		}
	} while (0);
}


void sirius::library::cache::local::storage::manager::on_ftell(const char * hash, int32_t fsize)
{
	std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t> cindex = NULL;
	{
		sirius::shared_scopedlock lock(&_clock);
		std::map<std::string, std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t>>::iterator citer = _cache.find(hash);
		if (citer != _cache.end())
			cindex = citer->second;
	}

	if (cindex)
	{
		sirius::exclusive_scopedlock lock(&cindex->lock);
		cindex->size = fsize;
	}
}

HANDLE sirius::library::cache::local::storage::manager::create_cache_file(int32_t width, int32_t height, const char * hash)
{
	BOOL available = TRUE;
	char filename[MAX_PATH] = { 0 };
	char filepath[MAX_PATH] = { 0 };
	_snprintf_s(filepath, MAX_PATH, "%s\\%dx%d", _cache_path, width, height);
	if (_access(filepath, 0) != 0)
	{
		available = ::CreateDirectoryA(filepath, NULL);
	}

	if (available)
	{
		_snprintf_s(filename, MAX_PATH, "%s\\%dx%d\\%s.png", _cache_path, width, height, hash);
		HANDLE file = ::CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		return file;
	}
	else
	{
		return INVALID_HANDLE_VALUE;
	}
}

BOOL sirius::library::cache::local::storage::manager::do_shared_writing(HANDLE file, int32_t offset, const char * packet, int32_t size)
{
	if (file != NULL && file != INVALID_HANDLE_VALUE)
	{
		::SetFilePointer(file, offset, NULL, FILE_BEGIN);

		BOOL bwritten = FALSE;
		uint32_t nwritten = 0;
		do
		{
			uint32_t nb_write = 0;
			bwritten = ::WriteFile(file, packet + nwritten, size - nwritten, (LPDWORD)&nb_write, 0);
			if (!bwritten)
				break;
			nwritten += nb_write;
			if (size == nwritten)
				break;
		} while (1);
		return bwritten;
	}
	else
	{
		return FALSE;
	}
}

int32_t sirius::library::cache::local::storage::manager::create_serialized_cached_index(std::shared_ptr<sirius::library::cache::local::storage::manager::uploading_state_t> us)
{
	int32_t status = sirius::library::cache::local::storage::manager::err_code_t::fail;
	do
	{
		std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t> cindex = NULL;
		{
			sirius::shared_scopedlock lock(&_clock);
			std::map<std::string, std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t>>::iterator iter = _cache.find(us->hash);
			if (iter != _cache.end())
				cindex = iter->second;
		}

		if (cindex)
		{
			sirius::exclusive_scopedlock lock(&cindex->lock);
			if (!cindex->serialized && cindex->upload_needed)
			{
				cindex->size = us->size;
				cindex->width = us->width;
				cindex->height = us->height;
				cindex->upload_needed = FALSE;
				cindex->serialized = TRUE;

				//::OutputDebugStringA("serialized\n");
			}
		}

		status = sirius::library::cache::local::storage::manager::err_code_t::success;

	} while (0);

	return status;
}

BOOL sirius::library::cache::local::storage::manager::read(HANDLE file, int64_t offset, char * packet, int32_t size)
{
	::SetFilePointer(file, ((uint32_t*)&offset)[0], (PLONG)&((uint32_t*)&offset)[1], FILE_BEGIN);

	DWORD bytes2read = size;
	DWORD bytesread = 0;
	BOOL bio = FALSE;
	BOOL eof = FALSE;
	do
	{
		DWORD nbread = 0;
		bio = ::ReadFile(file, packet + bytesread, bytes2read - bytesread, &nbread, NULL);
		if (!bio)
			break;
		if (nbread == 0)
		{
			eof = TRUE;
			break;
		}
		bytesread += nbread;
		if (bytes2read == bytesread)
			break;
	} while (1);

	if (eof)
		return FALSE;
	else
		return TRUE;
}

BOOL sirius::library::cache::local::storage::manager::read(HANDLE file, int64_t offset, int64_t & packet)
{
	::SetFilePointer(file, ((uint32_t*)&offset)[0], (PLONG)&((uint32_t*)&offset)[1], FILE_BEGIN);

	DWORD bytes2read = sizeof(int64_t);
	DWORD bytesread = 0;
	BOOL bio = FALSE;
	BOOL eof = FALSE;
	do
	{
		DWORD nbread = 0;
		bio = ::ReadFile(file, (&packet) + bytesread, bytes2read - bytesread, &nbread, NULL);
		if (!bio)
			break;
		if (nbread == 0)
		{
			eof = TRUE;
			break;
		}
		bytesread += nbread;
		if (bytes2read == bytesread)
			break;
	} while (1);

	if (eof)
		return FALSE;
	else
		return TRUE;
}

BOOL sirius::library::cache::local::storage::manager::read(HANDLE file, int64_t offset, long & packet)
{
	::SetFilePointer(file, ((uint32_t*)&offset)[0], (PLONG)&((uint32_t*)&offset)[1], FILE_BEGIN);

	DWORD bytes2read = sizeof(long);
	DWORD bytesread = 0;
	BOOL bio = FALSE;
	BOOL eof = FALSE;
	do
	{
		DWORD nbread = 0;
		bio = ::ReadFile(file, (&packet) + bytesread, bytes2read - bytesread, &nbread, NULL);
		if (!bio)
			break;
		if (nbread == 0)
		{
			eof = TRUE;
			break;
		}
		bytesread += nbread;
		if (bytes2read == bytesread)
			break;
	} while (1);

	if (eof)
		return FALSE;
	else
		return TRUE;
}

BOOL sirius::library::cache::local::storage::manager::read(HANDLE file, int64_t offset, int32_t & packet)
{
	::SetFilePointer(file, ((uint32_t*)&offset)[0], (PLONG)&((uint32_t*)&offset)[1], FILE_BEGIN);

	DWORD bytes2read = sizeof(int32_t);
	DWORD bytesread = 0;
	BOOL bio = FALSE;
	BOOL eof = FALSE;
	do
	{
		DWORD nbread = 0;
		bio = ::ReadFile(file, (&packet) + bytesread, bytes2read - bytesread, &nbread, NULL);
		if (!bio)
			break;
		if (nbread == 0)
		{
			eof = TRUE;
			break;
		}
		bytesread += nbread;
		if (bytes2read == bytesread)
			break;
	} while (1);

	if (eof)
		return FALSE;
	else
		return TRUE;
}

BOOL sirius::library::cache::local::storage::manager::write(HANDLE file, int64_t offset, char * packet, int32_t size)
{
	::SetFilePointer(file, ((uint32_t*)&offset)[0], (PLONG)&((uint32_t*)&offset)[1], FILE_BEGIN);

	BOOL bwritten = FALSE;
	uint32_t nwritten = 0;
	do
	{
		uint32_t nb_write = 0;
		bwritten = ::WriteFile(file, packet + nwritten, size - nwritten, (LPDWORD)&nb_write, 0);
		if (!bwritten)
			break;
		nwritten += nb_write;
		if (size == nwritten)
			break;
	} while (1);

	return TRUE;
}

BOOL sirius::library::cache::local::storage::manager::write(HANDLE file, int64_t offset, int64_t packet)
{
	::SetFilePointer(file, ((uint32_t*)&offset)[0], (PLONG)&((uint32_t*)&offset)[1], FILE_BEGIN);

	BOOL bwritten = FALSE;
	uint32_t nwritten = 0;
	int32_t size = sizeof(int64_t);
	do
	{
		uint32_t nb_write = 0;
		bwritten = ::WriteFile(file, (&packet) + nwritten, size - nwritten, (LPDWORD)&nb_write, 0);
		if (!bwritten)
			break;
		nwritten += nb_write;
		if (size == nwritten)
			break;
	} while (1);

	return TRUE;
}

BOOL sirius::library::cache::local::storage::manager::write(HANDLE file, int64_t offset, long packet)
{
	::SetFilePointer(file, ((uint32_t*)&offset)[0], (PLONG)&((uint32_t*)&offset)[1], FILE_BEGIN);

	BOOL bwritten = FALSE;
	uint32_t nwritten = 0;
	int32_t size = sizeof(long);
	do
	{
		uint32_t nb_write = 0;
		bwritten = ::WriteFile(file, (&packet) + nwritten, size - nwritten, (LPDWORD)&nb_write, 0);
		if (!bwritten)
			break;
		nwritten += nb_write;
		if (size == nwritten)
			break;
	} while (1);

	return TRUE;
}

BOOL sirius::library::cache::local::storage::manager::write(HANDLE file, int64_t offset, int32_t packet)
{
	::SetFilePointer(file, ((uint32_t*)&offset)[0], (PLONG)&((uint32_t*)&offset)[1], FILE_BEGIN);

	BOOL bwritten = FALSE;
	uint32_t nwritten = 0;
	int32_t size = sizeof(int32_t);
	do
	{
		uint32_t nb_write = 0;
		bwritten = ::WriteFile(file, (&packet) + nwritten, size - nwritten, (LPDWORD)&nb_write, 0);
		if (!bwritten)
			break;
		nwritten += nb_write;
		if (size == nwritten)
			break;
	} while (1);

	return TRUE;
}

unsigned __stdcall sirius::library::cache::local::storage::manager::process_cb(void * param)
{
	sirius::library::cache::local::storage::manager * self = static_cast<sirius::library::cache::local::storage::manager*>(param);
	self->process();
	return 0;
}

void sirius::library::cache::local::storage::manager::process(void)
{
	std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t> cindex = NULL;
	std::shared_ptr<sirius::library::cache::local::storage::manager::sorted_index_t> sindex = NULL;
	std::map<std::string, std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t>> cache;
	std::map<std::string, std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t>>::iterator citer;
#if defined(WIN64)
	std::map<int64_t, std::shared_ptr<sirius::library::cache::local::storage::manager::sorted_index_t>, std::greater<int64_t>> sorted;
	std::map<int64_t, std::shared_ptr<sirius::library::cache::local::storage::manager::sorted_index_t>>::iterator siter;
#else
	std::map<long, std::shared_ptr<sirius::library::cache::local::storage::manager::sorted_index_t>, std::greater<int64_t>> sorted;
	std::map<long, std::shared_ptr<sirius::library::cache::local::storage::manager::sorted_index_t>>::iterator siter;
#endif
	std::map<std::string, std::shared_ptr<sirius::library::cache::local::storage::manager::cached_index_t>>::iterator giter;
	while (_run)
	{
		{
			sirius::shared_scopedlock lock(&_clock);
			cache = _cache;
		}

		for (citer = cache.begin(); citer != cache.end(); citer++)
		{
			cindex = citer->second;
			if (cindex)
			{
				sirius::shared_scopedlock lock(&cindex->lock);

				siter = sorted.find(cindex->hitcount);
				if (siter != sorted.end())
				{
					sindex = siter->second;
				}
				else
				{
					sindex = std::shared_ptr<sirius::library::cache::local::storage::manager::sorted_index_t>(new sirius::library::cache::local::storage::manager::sorted_index_t);
					sorted.insert(std::make_pair(cindex->hitcount, sindex));
				}

				sindex->group.insert(std::make_pair(cindex->hash, cindex));
			}
		}


		int64_t current_cache_usage = 0;
		int64_t last_cache_usage = 0;
		BOOL cache_full = FALSE;
		for (siter = sorted.begin(); siter != sorted.end(); siter++)
		{
			sindex = siter->second;
			if (sindex)
			{
				for (giter = sindex->group.begin(); giter != sindex->group.end(); giter++)
				{
					cindex = giter->second;
					if (cindex)
					{
						if (!cache_full)
						{
							current_cache_usage += cindex->size;
							if (current_cache_usage == _cache_limit)
							{
								::OutputDebugStringA("cache full #1\n");
								sirius::exclusive_scopedlock lock(&cindex->lock);
								if (!cindex->serialized)
								{
									cindex->upload_needed = TRUE;
								}
								cache_full = TRUE;
							}
							else if (current_cache_usage > _cache_limit)
							{
								::OutputDebugStringA("cache full #2\n");
								cache_full = TRUE;
							}
							else
							{
								sirius::exclusive_scopedlock lock(&cindex->lock);
								if (!cindex->serialized)
								{
									cindex->upload_needed = TRUE;
								}
							}
						}
						else
						{
							sirius::exclusive_scopedlock lock(&cindex->lock);
							if (cindex->serialized)
							{
								cindex->serialized = FALSE;
								char filename[MAX_PATH] = { 0 };
								_snprintf_s(filename, MAX_PATH, "%s\\%dx%d\\%s.png", _cache_path, cindex->width, cindex->height, cindex->hash);
								if (PathFileExistsA(filename))
								{
									::DeleteFileA(filename);
									::OutputDebugStringA("DeleteFileA\n");
								}
							}
						}
					}
				}
			}
		}

		_cache_full = cache_full;

		sorted.clear();
		cache.clear();
		::Sleep(5);
	}
}
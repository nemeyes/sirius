#ifndef _BASE_CLIENT_H_
#define _BASE_CLIENT_H_

#include <pthread.h>
#include <sirius_circular_buffer.h>

class base_client
{
public:
	typedef struct _err_code_t
	{
		static const int success = 0;
		static const int fail = 1;
		static const int connection_error = 2;
		static const int closesocket_error = 3;
	} err_code_t;

	typedef struct _buffer_t
	{
		long long	timestamp;
		int			amount;
		_buffer_t * prev;
		_buffer_t * next;
	} buffer_t;

	class scopedlock
	{
	public:
		scopedlock(pthread_mutex_t * lock)
			: _lock(lock)
		{
			pthread_mutex_lock(_lock);
		}

		~scopedlock(void)
		{
			pthread_mutex_unlock(_lock);
		}
	private:
		pthread_mutex_t * _lock;
	};

	static const int MAX_SEND_BUFFER_SIZE = 1024 * 1024 * 2;

	base_client(int recv_buffer_size);
	virtual ~base_client(void);

	virtual int		start(const char * address, int portnumber);
	virtual int		stop(void);
	virtual int		send(const char * packet, int packet_size);
	virtual void	on_connect_to_server(void) = 0;
	virtual void	on_disconnect_from_server(void) = 0;
	virtual void	on_recv(const char * packet, int packet_size) = 0;

protected:
	int				connect(const char * address, int portnumber);
	int				disconnect(void);

	static void *	process_cb(void * param);
	void			process(void * self);


private:
	int				init(base_client::buffer_t * buffer);
	int				push(const char * data, int size, long long timestamp);
	int				pop(char * data, int & size, long long & timestamp);
	int				flush(void);

protected:
	pthread_mutex_t _lock;
	pthread_t		_thread;
	bool			_run;

	char			_address[200];
	int				_portnumber;
	int				_fd;

	sirius::circular::buffer_t *	_circular_buffer;
	base_client::buffer_t *			_send_circular_buffer;
	pthread_mutex_t					_send_buffer_lock;

	char *							_send_buffer;
	int								_send_buffer_size;

	char *							_recv_buffer;
	int								_recv_buffer_size;
};







#endif

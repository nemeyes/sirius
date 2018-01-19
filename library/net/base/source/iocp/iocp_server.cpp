#include <platform.h>
#include <iocp_server.h>
#include <sirius_log4cplus_logger.h>
#include <sirius_locks.h>

#if _MSC_VER >= 1900
FILE _iob[] = { *stdin, *stdout, *stderr };
extern "C" FILE * __cdecl __iob_func(void) { return _iob; }
#pragma comment (lib, "legacy_stdio_definitions.lib")
#endif

const char * sirius::library::net::iocp::server::_ca_cert_key_pem =
"Bag Attributes\n"
"    localKeyID: 25 3A 15 02 C8 DD E6 AD B2 DA F2 AD 89 84 6A 16 BA D1 B8 C5 \n"
"subject=/C=US/ST=CA/L=LA/O=Test Root CA/OU=IT/CN=www.testrootca.com\n"
"issuer=/C=US/ST=CA/L=LA/O=Test Root CA/OU=IT/CN=www.testrootca.com\n"
"-----BEGIN CERTIFICATE-----\n"
"MIIDozCCAougAwIBAgIJAPjAby1t84jRMA0GCSqGSIb3DQEBBQUAMGgxCzAJBgNV\n"
"BAYTAlVTMQswCQYDVQQIDAJDQTELMAkGA1UEBwwCTEExFTATBgNVBAoMDFRlc3Qg\n"
"Um9vdCBDQTELMAkGA1UECwwCSVQxGzAZBgNVBAMMEnd3dy50ZXN0cm9vdGNhLmNv\n"
"bTAeFw0xMzAyMjcxODE5MzFaFw0yMzAyMjUxODE5MzFaMGgxCzAJBgNVBAYTAlVT\n"
"MQswCQYDVQQIDAJDQTELMAkGA1UEBwwCTEExFTATBgNVBAoMDFRlc3QgUm9vdCBD\n"
"QTELMAkGA1UECwwCSVQxGzAZBgNVBAMMEnd3dy50ZXN0cm9vdGNhLmNvbTCCASIw\n"
"DQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAPp5lWbHbYPAzHFClxqukSMLwCfP\n"
"QuI/eR/VBwdVgGws4JfEkhQ+RB5flibnaxM0dovp21M/Hiayn68ZB8y/M89FLMzn\n"
"8kO0EiSgx01HXABEdzzzdUJcugXs+pRnSyBx7Ss38vvWu9qSZsE1q2b8TH6mQ29h\n"
"ZDFSxJwavcOiBJFDI4ktOWKf+eB7WIqG6Ria6v6fnfT3ed2PhaaK6s5FJULJZxc+\n"
"0tgLiKKxTuGzoJiSdlvYqN9BVZPBxtA42c/lmsk3YWc1jO79gXcwNf4gQFnn5Spb\n"
"zeEM3LGhJR7P2bAdGuc7kwjyEoq4lqrMgJMcKCgjIq1eVRKi1+7hdOlQjAECAwEA\n"
"AaNQME4wHQYDVR0OBBYEFOA4FxeNdBAXZELBXkVnT/eUC907MB8GA1UdIwQYMBaA\n"
"FOA4FxeNdBAXZELBXkVnT/eUC907MAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcNAQEF\n"
"BQADggEBAHvqLdE8Ga/U3pzlcbndjcyywcumiTimnx0QeazY/yHTs+qrza7gH224\n"
"CMuIyMdLbeFFylrTypuM5cU1KT7SV74nvdPUE2hmnXyq8lddhVWhpMp5aQLFnKxA\n"
"xyYEBuTtYqQLlBGlz4tZsa5Cb8C5XZTmlv32loQ3ho0bWACAwdoa3AmAzxHJkkkF\n"
"tMff2IzXtkLWrqjnfDLkFHeSHH07b9+rd4o92FWAX2JusmOWENNXyvFaL9YCdATz\n"
"36flXW4LTaJjumrfW+v2Ez0d1l6ZShoGh2/neG992i7gRAoEZQ0HDAsaTOpwbp4P\n"
"N/UU440JjdgBOJEhtuSmBiLCczAQN2c=\n"
"-----END CERTIFICATE-----\n"
"Bag Attributes\n"
"    localKeyID: 25 3A 15 02 C8 DD E6 AD B2 DA F2 AD 89 84 6A 16 BA D1 B8 C5 \n"
"Key Attributes: <No Attributes>\n"
"-----BEGIN PRIVATE KEY-----\n"
"MIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQD6eZVmx22DwMxx\n"
"QpcarpEjC8Anz0LiP3kf1QcHVYBsLOCXxJIUPkQeX5Ym52sTNHaL6dtTPx4msp+v\n"
"GQfMvzPPRSzM5/JDtBIkoMdNR1wARHc883VCXLoF7PqUZ0sgce0rN/L71rvakmbB\n"
"Natm/Ex+pkNvYWQxUsScGr3DogSRQyOJLTlin/nge1iKhukYmur+n53093ndj4Wm\n"
"iurORSVCyWcXPtLYC4iisU7hs6CYknZb2KjfQVWTwcbQONnP5ZrJN2FnNYzu/YF3\n"
"MDX+IEBZ5+UqW83hDNyxoSUez9mwHRrnO5MI8hKKuJaqzICTHCgoIyKtXlUSotfu\n"
"4XTpUIwBAgMBAAECggEBALIYl3GQJb+D7BowVg7DxzEbHtNvnMgQh8hpiWjek8TX\n"
"GA7x10iLiZXrM3GQJONv70bGzotfmsm7Qq2W56Pe5DDhp9w+pFUdibT/mRbKwZDz\n"
"HbF6DotUMXUbLoUs+Q8l0pRjYONOqR3d8TA6QBinFqELCXH91ub3ShMRNJMQ9Zxu\n"
"r44AGmiamulabXPtY3Yj9Fugz9iNl91SfQE2MCxuI+Kl2goJ6jj+N/LMQyKNxqVw\n"
"zIwXL6JWh/sX39n4Z5rW1BU2l4d+zufzfW47OPyhY7NSbO7Z2Rb9sS61ueiEOVj8\n"
"tadvWoimDOZLsfVrNNs1Mtmctd7uTW7EHyUYW5Gspd0CgYEA/uY3KsLLnAkHhxlg\n"
"7xoMXJ37cD2B5epMch6SItaTzx/23kSrmAyHPtu7VQ6I92HjiKQFutEhQIm61jYM\n"
"S0Ifkz1013yFZlkwma6z6ojU04xR0jJuHqNRPuHcgKcWFKwN0qMfqIC3oBU4ww/L\n"
"wIlBtBsDoMFkbvY8PViG/QN5h9cCgYEA+456I7G3hwRYS72HIVCyMkijdNBX3w4H\n"
"4U3XlKie1mdUfTtDT8luph7QitThk9S9UnTO2YAc30N7FmkdNFAysHzDkq2+rea8\n"
"k3qJoHDTgdQ3Zm1L8J9QFQtR8/cjzcnBRL0CKAzkWN4VsVoM3Lll6PrTd4H66y/N\n"
"KnXKIn4tr+cCgYAJQ7jRlw7rdvCNGoEJb7rvStaut62W+7MXmsDY/0+UAadpYLmT\n"
"5zCrOjVO3B5iT26DsuJ6bct1mwvtaT19JsQbB+rNQjtfAvLfQxhK9/o0Ti9TIn4a\n"
"Ivl6/Z6UFn6Y9b/y3fbymusRqHPNsy9GqjZ3I1w4ZY8O8WgzGpqda5jpNQKBgQCV\n"
"yBn1kuXCINYqEumY7qGLyzB4Z21KxOHwRxcrztL0uoXWIylLIpcdlo95epPuH+ey\n"
"Zxaa7utCDL4HdAMNmQr1hvQ0K/uzNAXokjfMLiHh1eg+tqlW1qUqQplDeE37RDwt\n"
"n5TfH/3+5A1qkkHCUynY9TBgoiYzTz0MISKAMz0k1QKBgQChAgr35Hszuq4Crxjj\n"
"Uu3YP7ETPZ5vfwDd6XaR9Ds+kG5EswQ1qVqFLY4Py1rmFImi2L64HYX83sS1ZtWh\n"
"ve8K2IrrDe58r+x8ktqkQdJ630TYhIKmNJ0SGxuLLEL0uqfZw23ErCfUouUer0IO\n"
"+k/jFXw0HAlo2uWUrly5rFjOsw==\n"
"-----END PRIVATE KEY-----\n";

const char * sirius::library::net::iocp::server::_server_cert_key_pem =
"Bag Attributes\n"
"    localKeyID: D4 99 A5 A4 0A 78 18 E6 AC EC 55 1D B4 FE 4A 83 27 82 05 DE \n"
"subject=/C=US/ST=CA/L=LA/O=Test Company/OU=IT/CN=www.testserver.com\n"
"issuer=/C=US/ST=CA/L=LA/O=Test Root CA/OU=IT/CN=www.testrootca.com\n"
"-----BEGIN CERTIFICATE-----\n"
"MIIDRDCCAiwCAf8wDQYJKoZIhvcNAQEFBQAwaDELMAkGA1UEBhMCVVMxCzAJBgNV\n"
"BAgMAkNBMQswCQYDVQQHDAJMQTEVMBMGA1UECgwMVGVzdCBSb290IENBMQswCQYD\n"
"VQQLDAJJVDEbMBkGA1UEAwwSd3d3LnRlc3Ryb290Y2EuY29tMB4XDTEzMDIyNzE4\n"
"MjE1NloXDTIzMDIyNTE4MjE1NlowaDELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAkNB\n"
"MQswCQYDVQQHDAJMQTEVMBMGA1UECgwMVGVzdCBDb21wYW55MQswCQYDVQQLDAJJ\n"
"VDEbMBkGA1UEAwwSd3d3LnRlc3RzZXJ2ZXIuY29tMIIBIjANBgkqhkiG9w0BAQEF\n"
"AAOCAQ8AMIIBCgKCAQEAzkHv+S30g5Dc+F1RJ1PUq9Hbh1YkEUJdYEj7ti+UfONV\n"
"NOT24hXzg8zaNSVO2Bhm+l8vzOVYMnjK9xcGSq5R5I633+lEeFdxURfsSJv9Vymq\n"
"tHUj5eNkmjzWBVrf4HvnZTJtRJljs941zYUgyJT9tkQXaerGFKJ6sfdXYfhGrkuK\n"
"gA1e71TwpRFYcfyYbQ3htENTh2CFBv7l5gjrITcmEJwpcU3U4nx4ZTr0IPLmV2kr\n"
"K8IJysY4dqgRcmduEI5ZgbYGkdG8L7QjggFXA6QNDPu8DfmXeeqS0gIffEm22bk7\n"
"b2fMnPfnFsJLsDdyhgrdYktnWhtZNij0y80tV4YCTwIDAQABMA0GCSqGSIb3DQEB\n"
"BQUAA4IBAQDMLn9VnUQt6BWx73J1lExYO/LWulMOnMR/WSVFy9dSwry+E807ekMY\n"
"WC8b3gpgDIqfkZjmttE9VtAdss2Baten+oBW+K13339sxHvcn30OxOs/Bln0yvaZ\n"
"Be+Zir7iE450b1IdYI98PMTSKgrK2e3vx/uUOCgG2yvs6/1v5rz5er/M1SQNzdMS\n"
"blelHWRQ1/ExwoUWBfIBkx/A4lTPmLgoC9fnXSiLhHKbZdfCJD8KLzEV0Se+ocn/\n"
"vl+6tlcUznap0TsRQpC67T/NGUimxdAhb6G1/U6z9bq0QQIuDxpOIpvwIgLvfRFx\n"
"qZQxmxOcK28fejHngmek7ZJNYKQbNewP\n"
"-----END CERTIFICATE-----\n"
"Bag Attributes\n"
"    localKeyID: D4 99 A5 A4 0A 78 18 E6 AC EC 55 1D B4 FE 4A 83 27 82 05 DE \n"
"Key Attributes: <No Attributes>\n"
"-----BEGIN PRIVATE KEY-----\n"
"MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQDOQe/5LfSDkNz4\n"
"XVEnU9Sr0duHViQRQl1gSPu2L5R841U05PbiFfODzNo1JU7YGGb6Xy/M5VgyeMr3\n"
"FwZKrlHkjrff6UR4V3FRF+xIm/1XKaq0dSPl42SaPNYFWt/ge+dlMm1EmWOz3jXN\n"
"hSDIlP22RBdp6sYUonqx91dh+EauS4qADV7vVPClEVhx/JhtDeG0Q1OHYIUG/uXm\n"
"COshNyYQnClxTdTifHhlOvQg8uZXaSsrwgnKxjh2qBFyZ24QjlmBtgaR0bwvtCOC\n"
"AVcDpA0M+7wN+Zd56pLSAh98SbbZuTtvZ8yc9+cWwkuwN3KGCt1iS2daG1k2KPTL\n"
"zS1XhgJPAgMBAAECggEAIT83s27Y7yw2skI4hqJYsamOPW6BOdb8vjyFdoSM5uSu\n"
"I2yU7zSioCgxNEfjQaoNT2ZwihKd+OTHsrSfawJWaQUoVot/YfaWaX/1sm6Sk64/\n"
"uf733mKdIM+VoB9Z3xGZ5xIN0vT2wVOcUJiZBDwf+XVYYNZbP5BBPtaj20LuAcIZ\n"
"OmW9uigdXQkQ1dylUkRPitjJ92bbysrTr621JTBSmvKnF7ctcF/Ql6VfS5RcqzYI\n"
"6U1vozoFkjmUnExlYZHC6qKCFG73Z+IcC7ojdMpzMp4/EqiveV/9EVdFlLRB1YAa\n"
"tND93xU9mo7L26XQzy79Xf2dWRUgUvaJ/7EvLA1RoQKBgQD2ZhJ9ogqfQ0ahq0D6\n"
"5neZo6bPbckEKshv1GKR5ixnYpPp1kCIxM8oIzb9fOvTX4MOMeRzPJyrJNwhVgfY\n"
"otWLrvkNviGHXN0frmkdj/Y/WSWh7clzzwXmGbB/8NPG4yzREvQ8vhKBkAmZln6K\n"
"ICl8J5NxOxF6GgYJ793GcsfZVQKBgQDWS3DYMVQ3eRgFajkQ/8+Gacgdu+8/SyM1\n"
"WptHOlPvKfqg3nZYPlAjMnVmk0Q7l/d2EtFBPP07/Jz0IvC/pMz0S8XfW/NigcRn\n"
"0R5Nci3BXbmQEjxNGt0m0sX4C4/Bx8ei8pugipX96OemT/bWP05RskL6tWsofGsb\n"
"8zgIQcldEwKBgCyx90iyzBp3qahJ2E+q3qcP+IJH9965pAIlFHxCtGtMhmg0ZSBq\n"
"EunE+YSh1GVTPgKlKjt9Ey44UXX6lRHG99WOt762bn6Pac0FZivmoVR8Z0coSxKm\n"
"yvsiTdHnbYL2UnraZVNfZxv5dMRXeDy1+NB8nVI81L7BWbcTu7bzuyzBAoGAY0j4\n"
"s3HHbxwvwPKCFhovcDs6eGxGYLDTUzjzkIC5uqlccYQgmKnmPyh1tFyu1F2ITbBS\n"
"O0OioFRd887sdB5KxzUELIRRs2YkNWVyALfR8zEVdGa+gYrcw8wL5OyWYlXJbPmy\n"
"mSMcc1OhYDDUUFdsVfWdisLbLxrWFVEOuOSiAvkCgYEA2viHsxoFxOrhnZQOhaLT\n"
"RPrgaSojv9pooHQ6fJwplewt91tb1OchDIeZk9Sl1hqPAXB0167of43GDOw2vfnq\n"
"Ust7RtiyJhQhSkz0qp4aH4P9l+dZJIWnpgjcyWkcz893br9gEuVnQgh13V/lcxOn\n"
"JtpaCFuHNTU3PcFiuQW+cN0=\n"
"-----END PRIVATE KEY-----\n";

int32_t				sirius::library::net::iocp::server::_nssl_locks = 0;
CRITICAL_SECTION *	sirius::library::net::iocp::server::_ssl_locks = nullptr;
SSL_CTX *			sirius::library::net::iocp::server::_ssl_ctx = nullptr;

sirius::library::net::iocp::server::server(int32_t so_recv_buffer_size, int32_t so_send_buffer_size, int32_t recv_buffer_size, int32_t send_buffer_size, BOOL tls)
	: _so_recv_buffer_size(so_recv_buffer_size)
	, _so_send_buffer_size(so_send_buffer_size)
	, _recv_buffer_size(recv_buffer_size)
	, _send_buffer_size(send_buffer_size)
	, _run(FALSE)
	, _thread(INVALID_HANDLE_VALUE)
	, _tls(tls)
{
	_iocp = new handler(this);
	memset(_address, 0x00, sizeof(_address));
}

sirius::library::net::iocp::server::~server(void)
{
	if (_iocp)
		delete _iocp;
	_iocp = nullptr;
}

int32_t sirius::library::net::iocp::server::initialize(void)
{
	WSADATA wsd;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
		return sirius::library::net::iocp::server::err_code_t::fail;
	return sirius::library::net::iocp::server::err_code_t::success;
}

int32_t sirius::library::net::iocp::server::release(void)
{
	WSACleanup();
	return sirius::library::net::iocp::server::err_code_t::success;
}

int32_t sirius::library::net::iocp::server::start(char * address, int32_t portnumber, int32_t io_thread_pool_count)
{
	if (address && strlen(address) > 0)
		strncpy_s(_address, address, MAX_PATH);
	_portnumber = portnumber;
	_io_thread_pool_count = io_thread_pool_count;

	unsigned int thread_id;
	_run = TRUE;
	_thread = (HANDLE)_beginthreadex(NULL, 0, process_cb, this, 0, &thread_id);
	if (_thread == NULL || _thread == INVALID_HANDLE_VALUE)
		return sirius::library::net::iocp::server::err_code_t::fail;

	return sirius::library::net::iocp::server::err_code_t::success;
}

int32_t sirius::library::net::iocp::server::stop(void)
{
	if (_thread != NULL && _thread != INVALID_HANDLE_VALUE)
	{
		_run = FALSE;
		if (::WaitForSingleObject(_thread, INFINITE) == WAIT_OBJECT_0)
			::CloseHandle(_thread);
	}

	return sirius::library::net::iocp::server::err_code_t::success;
}

void sirius::library::net::iocp::server::accept_session(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	_accept_session = session;
}

std::shared_ptr<sirius::library::net::iocp::session> sirius::library::net::iocp::server::accept_session(void)
{
	return _accept_session;
}

BOOL sirius::library::net::iocp::server::active(void) const
{
	return _run;
}

BOOL sirius::library::net::iocp::server::associate(SOCKET socket, ULONG_PTR key, int32_t * err_code)
{
	return _iocp->associate(socket, key, err_code);
}

void sirius::library::net::iocp::server::data_request(std::shared_ptr<sirius::library::net::iocp::session> session, const char * packet, int32_t packet_size)
{
	session->send(packet, packet_size);
}

void sirius::library::net::iocp::server::on_session_handshaking(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	session->recv(session->recv_context()->packet_capacity);
	on_app_session_handshaking(session);
}

void sirius::library::net::iocp::server::on_session_connect(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	if (_tls)
		session->recv(session->recv_context()->packet_capacity);
	else
		session->recv(session->packet_header_size());	
	on_app_session_connect(session);
}

void sirius::library::net::iocp::server::on_session_close(std::shared_ptr<sirius::library::net::iocp::session> session)
{
	on_app_session_close(session);
	destroy_session(session);
}

void sirius::library::net::iocp::server::execute(void)
{
	while (TRUE)
	{
		ULONG_PTR		completion_key	= 0;
		LPOVERLAPPED	overlapped		= 0;
		DWORD			nbytes			= 0;
		int32_t			err_code		= 0;

		BOOL value = _iocp->get_completion_status(&completion_key, &nbytes, &overlapped, &err_code);
		if (completion_key == NULL && overlapped == NULL)
		{
			::OutputDebugStringA("post_completion_status\n");
			_iocp->post_completion_status(0, 0, 0, &err_code);
			break;
		}

		::OutputDebugStringA("execute\n");

		sirius::library::net::iocp::session::io_context_t * p = reinterpret_cast<sirius::library::net::iocp::session::io_context_t*>(overlapped);
		std::shared_ptr<sirius::library::net::iocp::session::io_context_t> io_context = p->shared_from_this();
		if (io_context)
		{
			if (io_context->session)
			{
				std::shared_ptr<sirius::library::net::iocp::session> session = io_context->session->shared_from_this();
				if(session)
					session->on_completed(nbytes, overlapped);
			}
		}
	}
}

void sirius::library::net::iocp::server::initialization_tls(void)
{
	_nssl_locks = CRYPTO_num_locks();
	if (_nssl_locks > 0)
	{
		_ssl_locks = (CRITICAL_SECTION*)malloc(_nssl_locks * sizeof(CRITICAL_SECTION));
		for (int32_t n = 0; n < _nssl_locks; ++n)
			InitializeCriticalSection(&_ssl_locks[n]);
	}

#ifdef _DEBUG
	CRYPTO_malloc_debug_init();
	CRYPTO_dbg_set_options(V_CRYPTO_MDEBUG_ALL);
	CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON);
#endif

	CRYPTO_set_locking_callback(&ssl_lock_callback);
	CRYPTO_set_dynlock_create_callback(&ssl_lock_dyn_create_callback);
	CRYPTO_set_dynlock_lock_callback(&ssl_lock_dyn_callback);
	CRYPTO_set_dynlock_destroy_callback(&ssl_lock_dyn_destroy_callback);

	SSL_load_error_strings();
	SSL_library_init();

	const SSL_METHOD * meth = TLSv1_2_method(); //SSLv23_method();

	_ssl_ctx = SSL_CTX_new(meth);
	SSL_CTX_set_verify(_ssl_ctx, SSL_VERIFY_NONE, nullptr);
}

void sirius::library::net::iocp::server::release_tls(void)
{
	SSL_CTX_free(_ssl_ctx);
	_ssl_ctx = nullptr;

	CRYPTO_set_locking_callback(NULL);
	CRYPTO_set_dynlock_create_callback(NULL);
	CRYPTO_set_dynlock_lock_callback(NULL);
	CRYPTO_set_dynlock_destroy_callback(NULL);

	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();
	ERR_remove_state(0);
	ERR_free_strings();

	if (nullptr != _ssl_locks)
	{
		for (int n = 0; n < _nssl_locks; ++n)
			DeleteCriticalSection(&_ssl_locks[n]);

		free(_ssl_locks);
		_ssl_locks = nullptr;
		_nssl_locks = 0;
	}
}

void sirius::library::net::iocp::server::set_certificate(void)
{
	int length = strlen(_server_cert_key_pem);
	BIO *bio_cert = BIO_new_mem_buf((void*)_server_cert_key_pem, length);
	X509 *cert = PEM_read_bio_X509(bio_cert, nullptr, nullptr, nullptr);
	//printf("Certificate used for server:\n");
	//ssl_print_cert_info(cert);
	EVP_PKEY *pkey = PEM_read_bio_PrivateKey(bio_cert, 0, 0, 0);
	SSL_CTX_use_certificate(_ssl_ctx, cert);
	SSL_CTX_use_PrivateKey(_ssl_ctx, pkey);
	X509_free(cert);
	EVP_PKEY_free(pkey);
	BIO_free(bio_cert);
}

void sirius::library::net::iocp::server::ssl_lock_callback(int mode, int n, const char * file, int line)
{
	if (mode & CRYPTO_LOCK)
		::EnterCriticalSection(&_ssl_locks[n]);
	else
		::LeaveCriticalSection(&_ssl_locks[n]);
}

CRYPTO_dynlock_value * sirius::library::net::iocp::server::ssl_lock_dyn_create_callback(const char * file, int line)
{
	CRYPTO_dynlock_value * l = (CRYPTO_dynlock_value*)malloc(sizeof(CRYPTO_dynlock_value));
	::InitializeCriticalSection(&l->lock);
	return l;
}

void sirius::library::net::iocp::server::ssl_lock_dyn_callback(int mode, CRYPTO_dynlock_value * l, const char * file, int line)
{
	if (mode & CRYPTO_LOCK)
		::EnterCriticalSection(&l->lock);
	else
		::LeaveCriticalSection(&l->lock);
}

void sirius::library::net::iocp::server::ssl_lock_dyn_destroy_callback(CRYPTO_dynlock_value * l, const char * file, int line)
{
	::DeleteCriticalSection(&l->lock);
	free(l);
}

SOCKET sirius::library::net::iocp::server::create_listen_socket(int32_t portnumber)
{
	sockaddr_storage addr = { 0 };

	SOCKADDR_IN sock_addr;
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons((short)portnumber);
	if (strlen(_address)<1)
		sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		sock_addr.sin_addr.s_addr = inet_addr(_address);

	SOCKET s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	int32_t resueaddr_enable = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&resueaddr_enable, sizeof(resueaddr_enable)) < 0)
	{
		closesocket(s);
		s = INVALID_SOCKET;
		return s;
	}

	int32_t err_code;
	if (!_iocp->associate(s, (ULONG_PTR)this, &err_code))
	{
		closesocket(s);
		s = INVALID_SOCKET;
		return s;
	}

	if (bind(s, (LPSOCKADDR)&sock_addr, sizeof(sock_addr)))
	{
		closesocket(s);
		s = INVALID_SOCKET;
		return s;
	}

	if (listen(s, SOMAXCONN) == SOCKET_ERROR)
	{
		closesocket(s);
		s = INVALID_SOCKET;
		return s;
	}
	return s;
}

unsigned __stdcall sirius::library::net::iocp::server::process_cb(void * param)
{
	sirius::library::net::iocp::server * self = static_cast<sirius::library::net::iocp::server*>(param);
	self->process();
	return 0;
}

void sirius::library::net::iocp::server::process(void)
{
	int32_t err_code;
	if (!_iocp->create(_io_thread_pool_count, &err_code))
		return;
	_iocp->create_thread_pool();

	on_start();

	if (_tls)
	{
		initialization_tls();
		set_certificate();
	}

	SOCKET s = create_listen_socket(_portnumber);
	_accept_session = create_session(_so_recv_buffer_size, _so_send_buffer_size, _recv_buffer_size, _send_buffer_size, _tls, _ssl_ctx);
	_accept_session->listen_socket(s);
	_accept_session->accept();

	on_running();


	if (_tls)
	{
		release_tls();
	}

	if (_iocp)
		_iocp->close_thread_pool();
	_iocp->destroy();

	closesocket(s);

	on_stop();
}
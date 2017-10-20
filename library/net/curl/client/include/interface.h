#pragma once


//status code
typedef struct _status_t {
	//요청 메세지인 경우
	static const int req_msg = 0;					
	//응답 메세지인 경우
	static const int err_none = 0;					//정상 응답인 경우
	static const int err_not_post_method = 100;		//POST방식의 요청이 아닌 경우
	static const int err_no_req_data = 200;			//요청 데이터가 없는 경우
	static const int err_xml_format = 200;			//text/xml형식이 아닌 경우
	static const int err_req_xml = 300;				//요청 xml에 오류가 있는 경우
	static const int err_csr_internal = 400;		//응답 xml 생성 중 오류가 발생한 경우
	static const int err_not_proper_csr_info = 500;	//잘못된 CSS/CSR의 정보를 사용하여 요청한 경우
	static const int err_connection_full = 600;		//모든 CSS의 Connection이 가득 찬 경우
	static const int err_db = 700;					//Database 처리 중 오류가 발생한 경우
	static const int err_etc = 900;					//기타 연동 장애
}status_t;

#pragma once


//status code
typedef struct _status_t {
	//��û �޼����� ���
	static const int req_msg = 0;					
	//���� �޼����� ���
	static const int err_none = 0;					//���� ������ ���
	static const int err_not_post_method = 100;		//POST����� ��û�� �ƴ� ���
	static const int err_no_req_data = 200;			//��û �����Ͱ� ���� ���
	static const int err_xml_format = 200;			//text/xml������ �ƴ� ���
	static const int err_req_xml = 300;				//��û xml�� ������ �ִ� ���
	static const int err_csr_internal = 400;		//���� xml ���� �� ������ �߻��� ���
	static const int err_not_proper_csr_info = 500;	//�߸��� CSS/CSR�� ������ ����Ͽ� ��û�� ���
	static const int err_connection_full = 600;		//��� CSS�� Connection�� ���� �� ���
	static const int err_db = 700;					//Database ó�� �� ������ �߻��� ���
	static const int err_etc = 900;					//��Ÿ ���� ���
}status_t;

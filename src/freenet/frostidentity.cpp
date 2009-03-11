#include "../../include/freenet/frostidentity.h"
#include "../../include/stringfunctions.h"
#include "../../include/base64.h"

#include <cstring>

FrostIdentity::FrostIdentity()
{
	std::memset(&m_rsa,0,sizeof(m_rsa));
}

FrostIdentity::~FrostIdentity()
{
	rsa_free(&m_rsa);
}

const bool FrostIdentity::FromPublicKey(const std::string &publickey)
{
	std::vector<std::string> keyparts;
	std::vector<unsigned char> edata;
	std::vector<unsigned char> ndata;

	rsa_free(&m_rsa);
	std::memset(&m_rsa,0,sizeof(m_rsa));

	StringFunctions::Split(publickey,":",keyparts);

	if(keyparts.size()==2)
	{
		Base64::Decode(keyparts[0],edata);
		Base64::Decode(keyparts[1],ndata);

		m_rsa.type=PK_PUBLIC;
#ifdef LTC_SOURCE
		mp_init(&m_rsa.N);
		mp_init(&m_rsa.e);
		mp_read_unsigned_bin(m_rsa.N,&ndata[0],ndata.size());
		mp_read_unsigned_bin(m_rsa.e,&edata[0],edata.size());
#else
		ltm_desc.init(&m_rsa.N);
		ltm_desc.init(&m_rsa.e);
		ltm_desc.unsigned_read(m_rsa.N,&ndata[0],ndata.size());
		ltm_desc.unsigned_read(m_rsa.e,&edata[0],edata.size());	
#endif

		m_publickey=publickey;

		return true;
	}
	else
	{
		return false;
	}
}

const bool FrostIdentity::VerifyAuthor(const std::string &author)
{
	std::vector<std::string> authorparts;
	std::vector<unsigned char> authorhash(100,0);
	unsigned long authorhashlen=authorhash.size();
	std::string authorhashstr="";
	std::vector<unsigned char> publickeydata(m_publickey.begin(),m_publickey.end());

	StringFunctions::Split(author,"@",authorparts);

	if(m_publickey!="" && authorparts.size()==2)
	{
		hash_memory(find_hash("sha1"),&publickeydata[0],publickeydata.size(),&authorhash[0],&authorhashlen);
		authorhash.resize(authorhashlen);

		Base64::Encode(authorhash,authorhashstr);

		authorhashstr.erase(27);
		authorhashstr=StringFunctions::Replace(authorhashstr,"/","_");

		return (authorhashstr==authorparts[1]);

	}
	else
	{
		return false;
	}

}

const bool FrostIdentity::VerifySignature(const std::vector<unsigned char> &data, const std::string &signature)
{
	std::vector<unsigned char> sigdata;
	std::vector<unsigned char> hashdata(100,0);
	unsigned long hashlen=hashdata.size();
	int status,rval;

	rval=status=0;

	Base64::Decode(signature,sigdata);

	hash_memory(find_hash("sha1"),&data[0],data.size(),&hashdata[0],&hashlen);
	hashdata.resize(hashlen);

	rval=rsa_verify_hash_ex(&sigdata[0],sigdata.size(),&hashdata[0],hashdata.size(),LTC_PKCS_1_PSS,find_hash("sha1"),16,&status,&m_rsa);

	return (rval==0 && status==1) ? true : false;

}

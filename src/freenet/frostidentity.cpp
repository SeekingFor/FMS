#include "../../include/freenet/frostidentity.h"
#include "../../include/stringfunctions.h"
#include "../../include/base64.h"

#include <polarssl/bignum.h>
#include <polarssl/sha1.h>
#include <polarssl/md.h>

#include <cstring>

FrostIdentity::FrostIdentity()
{
	rsa_init(&m_rsa,RSA_PKCS_V21,POLARSSL_MD_SHA1);
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
	rsa_init(&m_rsa,RSA_PKCS_V21,POLARSSL_MD_SHA1);

	StringFunctions::Split(publickey,":",keyparts);

	if(keyparts.size()==2)
	{
		Base64::Decode(keyparts[0],edata);
		Base64::Decode(keyparts[1],ndata);

		mpi_init(&m_rsa.N);
		mpi_init(&m_rsa.E);
		mpi_read_binary(&m_rsa.N,&ndata[0],ndata.size());
		mpi_read_binary(&m_rsa.E,&edata[0],edata.size());

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
	std::vector<unsigned char> authorhash(20,0);
	unsigned long authorhashlen=authorhash.size();
	std::string authorhashstr="";
	std::vector<unsigned char> publickeydata(m_publickey.begin(),m_publickey.end());

	StringFunctions::Split(author,"@",authorparts);

	if(m_publickey!="" && authorparts.size()==2)
	{
		sha1(&publickeydata[0],publickeydata.size(),&authorhash[0]);

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
	if(data.size()>0 && signature!="")
	{
		std::vector<unsigned char> sigdata;
		std::vector<unsigned char> hashdata(20,0);
		unsigned long hashlen=hashdata.size();
		int rval;

		rval=0;

		if(Base64::Decode(signature,sigdata)==true)
		{
			sha1(&data[0],data.size(),&hashdata[0]);

			m_rsa.len=sigdata.size();
			rval=rsa_pkcs1_verify(&m_rsa,RSA_PUBLIC,SIG_RSA_SHA1,hashdata.size(),&hashdata[0],&sigdata[0]);

			return (rval==0) ? true : false;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

}

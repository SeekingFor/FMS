#include "../../../include/http/pages/showavatarpage.h"
#include "../../../include/base64.h"

void ShowAvatarPage::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
	m_log->trace("ShowAvatarPage::handleRequest from "+request.clientAddress().toString());

	const int quadsize=20;
	const double quadsizedbl=20.0;
	std::map<std::string,QueryVar> queryvars;
	std::vector<unsigned char> bytes;

	CreateQueryVarMap(request,queryvars);

	if(request.getVersion()==Poco::Net::HTTPRequest::HTTP_1_1)
	{
		response.setChunkedTransferEncoding(true);
	}

	std::string content="";
	if(queryvars.find("idpart")!=queryvars.end())
	{
		std::string idpart=(*queryvars.find("idpart")).second.GetData();
		while(idpart.size()%4!=0)
		{
			idpart+="=";
		}
		Base64::Decode(Base64::FreenetBase64ToRealBase64(idpart),bytes);
		
		if(bytes.size()==32)
		{
			BMP bmp;
			bmp.SetBitDepth(32);
			bmp.SetSize(quadsize*4,quadsize*4);
			std::vector<RGBApixel> pixels(16);

			for(int i=0; i<16; i++)
			{
				// 5 bits red, 6 bits green, 5 bits blue
				pixels[i].Red=(bytes[(i*2)] & 0xf8);
				pixels[i].Green=((bytes[(i*2)] & 0x07) << 5) + ((bytes[(i*2)+1] & 0xe0) >> 3);
				pixels[i].Blue=((bytes[(i*2)+1] & 0x1f) << 3);
			}

			for(int quad=0; quad<16; quad++)
			{
				for(int yy=(quad/4)*quadsize; yy<quadsize+((quad/4)*quadsize); yy++)
				{
					for(int xx=(quad%4)*quadsize; xx<quadsize+((quad%4)*quadsize); xx++)
					{
						bmp.SetPixel(xx,yy,pixels[(quad%4)+((quad/4)*4)]);
					}
				}
			}

			std::vector<unsigned char> data;
			bmp.WriteToVector(data);

			response.setContentType("image/bmp");
			response.setContentLength(data.size());
			response.set("Content-Disposition","inline; filename=avatar.bmp");
			content+=std::string(data.begin(),data.end());
		}
	}
	std::ostream &ostr = response.send();
	ostr << content;

}

const RGBApixel ShowAvatarPage::Interpolate(const RGBApixel &first, const RGBApixel &second, const double val)
{
	RGBApixel rval;
	rval.Red=(static_cast<double>(first.Red)*(1.0-val))+(static_cast<double>(second.Red)*val);
	rval.Green=(static_cast<double>(first.Green)*(1.0-val))+(static_cast<double>(second.Green)*val);
	rval.Blue=(static_cast<double>(first.Blue)*(1.0-val))+(static_cast<double>(second.Blue)*val);
	rval.Alpha=(static_cast<double>(first.Alpha)*(1.0-val))+(static_cast<double>(second.Alpha)*val);
	return rval;
}

const bool ShowAvatarPage::WillHandleURI(const std::string &uri)
{
	if(uri.find("showavatar.")!=std::string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
}

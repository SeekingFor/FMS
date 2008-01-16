#include "../include/datetime.h"

#include <vector>
#include "../include/stringfunctions.h"

#ifdef XMEM
	#include <xmem.h>
#endif

DateTime::DateTime()
{
	Set();
}

DateTime::DateTime(const time_t &timet)
{
	Set(timet);
}

DateTime::DateTime(const struct tm *stm)
{
	Set(stm);
}

void DateTime::Add(const int seconds, const int minutes, const int hours, const int days, const int months, const int years)
{
	m_tm.tm_sec+=seconds;
	m_tm.tm_min+=minutes;
	m_tm.tm_hour+=hours;
	m_tm.tm_mday+=days;
	m_tm.tm_mon+=months;
	m_tm.tm_year+=years;
	
	Normalize();
}

std::string DateTime::Format(const std::string &formatstring) const
{
	std::string returnval="";
	char *str=new char[512];
	memset(str,0,512);

	strftime(str,511,formatstring.c_str(),&m_tm);

	if(str)
	{
		returnval=str;
		delete [] str;
	}

	return returnval;
}

void DateTime::Normalize()
{
	// check for tm_isdst in local time and take appropriate action when normalizing
	// thanks to http://www.erack.de/download/timetest.c for example
	int isdst;
	struct tm temptm;
	time_t temptimet;
	temptimet=time(NULL);
	temptm=*localtime(&temptimet);
	isdst=temptm.tm_isdst;

	temptm.tm_year=m_tm.tm_year;
	temptm.tm_mon=m_tm.tm_mon;
	temptm.tm_mday=m_tm.tm_mday;
	temptm.tm_hour=m_tm.tm_hour;
	temptm.tm_min=m_tm.tm_min;
	temptm.tm_sec=m_tm.tm_sec;
	temptm.tm_isdst=isdst;
	temptimet=mktime(&temptm);

	if(temptm.tm_isdst!=isdst)
	{
		// keep tm_isdst to whatever mktime returned and try again
		temptm.tm_year=m_tm.tm_year;
		temptm.tm_mon=m_tm.tm_mon;
		temptm.tm_mday=m_tm.tm_mday;
		temptm.tm_hour=m_tm.tm_hour;
		temptm.tm_min=m_tm.tm_min;
		temptm.tm_sec=m_tm.tm_sec;
		temptimet=mktime(&temptm);
	}
	else if(isdst && temptimet==-1)
	{
		// isdst set, but TZ has no offset (e.g. GMT), try with isdst=0
		temptm.tm_year=m_tm.tm_year;
		temptm.tm_mon=m_tm.tm_mon;
		temptm.tm_mday=m_tm.tm_mday;
		temptm.tm_hour=m_tm.tm_hour;
		temptm.tm_min=m_tm.tm_min;
		temptm.tm_sec=m_tm.tm_sec;
		temptm.tm_isdst=0;
		temptimet=mktime(&temptm);
	}

	m_tm=temptm;
	m_timet=temptimet;

	// date is erroneous - set to default date
	if(m_timet==-1 && (m_tm.tm_mon<0 || m_tm.tm_mon>11 || m_tm.tm_mday <1 || m_tm.tm_mday>31 || m_tm.tm_hour<0 || m_tm.tm_hour>23 || m_tm.tm_min<0 || m_tm.tm_min>59 || m_tm.tm_sec<0 || m_tm.tm_sec>59))
	{
		Set();
	}

}

DateTime DateTime::operator+(const double &rhs)
{
	DateTime temp=*this;

	double val=rhs;
	int days=(int)val;
	val-=days;
	val*=24.0;
	int hours=(int)val;
	val-=hours;
	val*=60.0;
	int minutes=(int)val;
	val-=minutes;
	val*=60.0;
	int seconds=(int)val;

	temp.Add(seconds,minutes,hours,days);
	return temp;
}

DateTime DateTime::operator+(const DateTime &rhs)
{
	DateTime temp=*this;

	temp.Add(rhs.m_tm.tm_sec,rhs.m_tm.tm_min,rhs.m_tm.tm_hour,rhs.m_tm.tm_mday,rhs.m_tm.tm_mon+1,rhs.m_tm.tm_year+1900);
	return temp;
}

DateTime &DateTime::operator+=(const double &rhs)
{
	*this=*this+rhs;

	return *this;
}

DateTime &DateTime::operator+=(const DateTime &rhs)
{
	*this=*this+rhs;

	return *this;
}

DateTime DateTime::operator-(const double &rhs)
{
	DateTime temp=*this;

	double val=rhs;
	int days=(int)val;
	val-=days;
	val*=24.0;
	int hours=(int)val;
	val-=hours;
	val*=60.0;
	int minutes=(int)val;
	val-=minutes;
	val*=60.0;
	int seconds=(int)val;

	temp.Add(-seconds,-minutes,-hours,-days);
	return temp;
}

DateTime DateTime::operator-(const DateTime &rhs)
{
	DateTime temp=*this;

	temp.Add(-rhs.m_tm.tm_sec,-rhs.m_tm.tm_min,-rhs.m_tm.tm_hour,-rhs.m_tm.tm_mday,-(rhs.m_tm.tm_mon+1),-(rhs.m_tm.tm_year+1900));
	return temp;
}

DateTime &DateTime::operator-=(const double &rhs)
{
	*this=*this-rhs;

	return *this;
}

DateTime &DateTime::operator-=(const DateTime &rhs)
{
	*this=*this-rhs;

	return *this;
}

const bool DateTime::operator==(const struct tm &rhs) const
{
	return (m_tm.tm_year==rhs.tm_year && m_tm.tm_mon==rhs.tm_mon && m_tm.tm_mday==rhs.tm_mday && m_tm.tm_hour==rhs.tm_hour && m_tm.tm_min==rhs.tm_min && m_tm.tm_sec==rhs.tm_sec) ? true : false;
}

void DateTime::Set(const int year, const int month, const int day, const int hour, const int minute, const int second)
{
	m_tm.tm_year=year-1900;
	m_tm.tm_mon=month-1;
	m_tm.tm_mday=day;
	m_tm.tm_hour=hour;
	m_tm.tm_min=minute;
	m_tm.tm_sec=second;

	Normalize();
}

void DateTime::Set(const time_t &timet)
{
	m_timet=timet;

	m_tm=*gmtime(&m_timet);
	Normalize();
}

void DateTime::Set(const struct tm *stm)
{
	m_tm=*stm;
	m_timet=mktime(&m_tm);
	Normalize();
}

void DateTime::Set(const std::string &datestring)
{
	int year,month,day,hour,minute,second;
	std::vector<std::string> tokens;
	int vecpos;
	int tempint;

	year=month=day=hour=minute=second=-1;

	// reset to 1900-01-01 00:00:00
	Set();

	StringFunctions::SplitMultiple(datestring,"-/\\., :",tokens);

	// loop through 1st time to try to find 4 digit year and month (if it is a text month)
	vecpos=0;
	for(std::vector<std::string>::iterator i=tokens.begin(); i!=tokens.end(); i++,vecpos++)
	{
		StringFunctions::UpperCase((*i),(*i));

		if((*i).find("JAN")==0)
		{
			SetMonth(1);
			month=vecpos;
		}
		if((*i).find("FEB")==0)
		{
			SetMonth(2);
			month=vecpos;
		}
		if((*i).find("MAR")==0)
		{
			SetMonth(3);
			month=vecpos;
		}
		if((*i).find("APR")==0)
		{
			SetMonth(4);
			month=vecpos;
		}
		if((*i).find("MAY")==0)
		{
			SetMonth(5);
			month=vecpos;
		}
		if((*i).find("JUN")==0)
		{
			SetMonth(6);
			month=vecpos;
		}
		if((*i).find("JUL")==0)
		{
			SetMonth(7);
			month=vecpos;
		}
		if((*i).find("AUG")==0)
		{
			SetMonth(8);
			month=vecpos;
		}
		if((*i).find("SEP")==0)
		{
			SetMonth(9);
			month=vecpos;
		}
		if((*i).find("OCT")==0)
		{
			SetMonth(10);
			month=vecpos;
		}
		if((*i).find("NOV")==0)
		{
			SetMonth(11);
			month=vecpos;
		}
		if((*i).find("DEC")==0)
		{
			SetMonth(12);
			month=vecpos;
		}

		// if we just got month - day is probaby in the next position
		if(month==vecpos && vecpos+1<tokens.size() && tokens[vecpos+1].size()>0)
		{
			tempint=-1;
			StringFunctions::Convert(tokens[vecpos+1],tempint);
			if(tempint>0 && tempint<32)
			{
				SetDay(tempint);
				day=vecpos+1;
			}
		}

		// if this is not month or day, and the size is 4 then it is probably the year
		if(month!=vecpos && day!=vecpos && (*i).size()==4)
		{
			tempint=-1;
			StringFunctions::Convert((*i),tempint);

			SetYear(tempint);
			year=vecpos;
		}
	}

	// month is probably right after year
	if(year!=-1 && month==-1 && year+1<tokens.size())
	{
		tempint=-1;
		StringFunctions::Convert(tokens[year+1],tempint);
		if(tempint>=1 && tempint<=12)
		{
			SetMonth(tempint);
			month=year+1;
		}
	}

	// otherwise it is probably 2 steps back (m/d/y)
	if(year!=-1 && month==-1 && year-2>=0)
	{
		tempint=-1;
		StringFunctions::Convert(tokens[year-2],tempint);
		if(tempint>=1 && tempint<=12)
		{
			SetMonth(tempint);
			month=year-2;
		}
	}

	// day is probably right after month
	if(month!=-1 && month+1<tokens.size())
	{
		tempint=-1;
		StringFunctions::Convert(tokens[month+1],tempint);
		if(tempint>=1 && tempint<32)
		{
			SetDay(tempint);
			day=month+1;
		}
	}

	// loop through another time to find hour
	vecpos=0;
	for(std::vector<std::string>::iterator i=tokens.begin(); i!=tokens.end(); i++,vecpos++)
	{
		if(vecpos!=year && vecpos!=month && vecpos!=day && hour==-1)
		{
			tempint=-1;
			StringFunctions::Convert((*i),tempint);
			if(tempint>=0 && tempint<24)
			{
				SetHour(tempint);
				hour=vecpos;
			}
		}
	}

	// minute right after hour
	if(hour!=-1 && hour+1<tokens.size())
	{
		tempint=-1;
		StringFunctions::Convert(tokens[hour+1],tempint);
		if(tempint>=0 && tempint<60)
		{
			SetMinute(tempint);
			minute=hour+1;
		}
	}

	//second right after minute
	if(minute!=-1 && minute+1<tokens.size())
	{
		tempint=-1;
		StringFunctions::Convert(tokens[minute+1],tempint);
		if(tempint>=0 && tempint<60)
		{
			SetSecond(tempint);
			second=minute+1;
		}
	}

}

void DateTime::SetToGMTime()
{
	m_timet=time(NULL);

	m_tm=*gmtime(&m_timet);
	Normalize();
}

void DateTime::SetToLocalTime()
{
	m_timet=time(NULL);
	m_tm=*localtime(&m_timet);
	Normalize();
}

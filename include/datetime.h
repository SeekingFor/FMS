#ifndef _date_time_
#define _date_time_

#include <ctime>
#include <string>

/*
	Year	actual year
	Month	1=Jan,2=Feb,etc
	Day		1 to last day of month
	Hour	0 to 23
	Minute	0 to 59
	Second	0 to 59

	WeekDay	0=Sunday,1=Monday,etc
	YearDay	1=Jan 1st, 2=Jan 2nd, etc

*/

class DateTime
{
public:
	DateTime();
	DateTime(const time_t &timet);
	DateTime(const struct tm *stm);
	~DateTime() {}

	void Add(const int seconds=0, const int minutes=0, const int hours=0, const int days=0, const int months=0, const int years=0);
	
	void Set(const int year=1970, const int month=1, const int day=1, const int hour=0, const int minute=0, const int second=0);
	void Set(const time_t &timet);
	void Set(const struct tm *stm);
	void Set(const std::string &datestring);	// method only will work with a select few basic input formats
	
	void SetToLocalTime();
	void SetToGMTime();
	
	const int GetYear() const			{ return m_tm.tm_year+1900; }
	void SetYear(const int year)		{ m_tm.tm_year=year-1900; }
	const int GetMonth() const			{ return m_tm.tm_mon+1; }
	void SetMonth(const int month)		{ m_tm.tm_mon=month-1; }
	const int GetDay() const			{ return m_tm.tm_mday; }
	void SetDay(const int day)			{ m_tm.tm_mday=day; }
	const int GetWeekDay() const		{ return m_tm.tm_wday; }
	void SetWeekDay(const int weekday)	{ m_tm.tm_wday=weekday; }
	const int GetYearDay() const		{ return m_tm.tm_yday+1; }
	void SetYearDay(const int yearday)	{ m_tm.tm_yday=yearday-1; }
	const int GetHour() const			{ return m_tm.tm_hour; }
	void SetHour(const int hour)		{ m_tm.tm_hour=hour; }
	const int GetMinute() const			{ return m_tm.tm_min; }
	void SetMinute(const int minute)	{ m_tm.tm_min=minute; }
	const int GetSecond() const			{ return m_tm.tm_sec; }
	void SetSecond(const int second)	{ m_tm.tm_sec=second; }
	const int GetIsDaylightTime() const	{ return m_tm.tm_isdst; }
	void SetIsDaylightTime(const int daylighttime) { m_tm.tm_isdst=daylighttime; }

	void Normalize();			// normalizes any date changes that were made
	
	std::string Format(const std::string &formatstring) const;

	DateTime operator+(const double &rhs);
	DateTime operator+(const DateTime &rhs);
	DateTime &operator+=(const double &rhs);
	DateTime &operator+=(const DateTime &rhs);
	DateTime operator-(const double &rhs);
	DateTime operator-(const DateTime &rhs);
	DateTime &operator-=(const double &rhs);
	DateTime &operator-=(const DateTime &rhs);
	
	const bool operator==(const DateTime &rhs) const { return m_timet==rhs.m_timet; }
	const bool operator==(const time_t &rhs) const { return m_timet==rhs; }
	const bool operator==(const struct tm &rhs) const;
	
	const bool operator<(const DateTime &rhs) const { return (m_timet<rhs.m_timet); }
	const bool operator<(const time_t &rhs) const { return (m_timet<rhs); }
	
	const bool operator<=(const DateTime &rhs) const { return (*this<rhs || *this==rhs); }
	const bool operator<=(const time_t &rhs) const { return (m_timet<=rhs); }
	
	const bool operator>(const DateTime &rhs) const { return (m_timet>rhs.m_timet); }
	const bool operator>(const time_t &rhs) const { return (m_timet>rhs); }
	
	const bool operator>=(const DateTime &rhs) const { return (*this>rhs || *this==rhs); }
	const bool operator>=(const time_t &rhs) const { return (m_timet>=rhs); }
	
private:

	time_t m_timet;
	struct tm m_tm;
};

#endif	// _date_time_

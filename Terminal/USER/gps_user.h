#ifndef __GPS_USER_H
#define __GPS_USER_H

unsigned char GetLocation( unsigned char* dsr );
extern int GetSpeed(void);
unsigned char isSubtend(unsigned char dir1,unsigned char dir2);
unsigned int GetDistance(unsigned int lat1,unsigned int lng1,unsigned int lat2,unsigned int lng2);

#endif


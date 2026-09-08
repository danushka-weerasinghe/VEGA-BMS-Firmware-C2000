/*
 * charge_profile.h
 *
 *  Created on: Nov 27, 2023
 *      Author: AsithDev
 */

#ifndef HEADERS_CHARGE_PROFILE_H_
#define HEADERS_CHARGE_PROFILE_H_


#define t_95_100(c)   c*0.05*60/25
#define t_85_100(c)   (t_95_100(c) + c*0.1*60/50)
#define t_15_100(c)   (t_85_100(c) + c*0.7*60/100)
#define t_5_100(c)    (t_85_100(c) + c*0.1*60/50)

#define t_15_80(c)   (c*0.65*60/100)
#define t_5_80(c)    (t_15_80(c) + c*0.1*60/50)




#endif /* HEADERS_CHARGE_PROFILE_H_ */

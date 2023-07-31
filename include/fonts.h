/**
 ******************************************************************************
 * @file fonts.h
 * @author GX (2524913364@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2022-07-07
 *
 * @copyright Copyright (c) 2022
 *
 ******************************************************************************
 *----------------------------------------------------------------------------*
 *  Remark         : Description                                              *
 *----------------------------------------------------------------------------*
 *  Change History :                                                          *
 *  <Date>     | <Version> | <Author>       | <Description>                   *
 *----------------------------------------------------------------------------*
 *  2022-07-07 | 1.0.0     | GX             | Create file                     *
 *----------------------------------------------------------------------------*
 *                                                                            *
 ******************************************************************************
 */


/*----------------------------- start of file -------------------------------*/
#ifndef _FONTS_H
#define _FONTS_H
#include <sys/types.h>


typedef struct _tFont
{
  const u_int8_t *table;
  u_int16_t Width;
  u_int16_t Height;

} sFONT;

// typedef enum FONT_SIZE
// {
//   Font24x32,
//   Font16x24,
//   Font8x16
// }t_FONT_SIZE;

extern sFONT Font48x64;
extern sFONT Font32x48;
extern sFONT Font24x32;
extern sFONT Font16x24;
extern sFONT Font8x16;



#endif  /* _FONTS_H */


/*------------------------------ end of file --------------------------------*/


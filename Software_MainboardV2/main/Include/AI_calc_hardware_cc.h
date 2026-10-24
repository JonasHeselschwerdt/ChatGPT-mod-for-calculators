/*

ChatGPT Hardware Hack for calculators: Software V2

© 2026 Jonas Heselschwerdt
Licensed under GPLv3 

hardware_cc: Hardware compile control file

*/

/*
#########################################################################################
##                                                                                     ##
##   Select the used camera by removing // (only 1)                                    ##
##                                                                                     ##
#########################################################################################
*/

//#define CC_CAMERA_USED_OV5640
//#define CC_CAMERA_USED_OV3660


/*
#########################################################################################
##                                                                                     ##
##   Check legitimacy of configuration                                                 ##
##                                                                                     ##
#########################################################################################
*/

#if defined(CC_CAMERA_USED_OV5640) && defined(CC_CAMERA_USED_OV3660)
    #error "Multiple camera models selected!"
#elif !defined(CC_CAMERA_USED_OV5640) && !defined(CC_CAMERA_USED_OV3660)
    #error "No camera model selected! Select OV5640 or OV3660."
#endif

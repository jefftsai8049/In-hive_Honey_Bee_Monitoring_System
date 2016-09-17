#ifndef PARAMETERS
#define PARAMETERS

#define BEE_TRACK_COUNT 100

//for image input size
#define IMAGE_SIZE_X 1200
#define IMAGE_SIZE_Y 1600

//for temperature and humidity recorder
#define SERIAL_TIME 2000
#define RECORD_TIME 60*10*1000

//for trajectory tracking and pattern classifier
#define REMAIN_SIZE 20
#define FORGET_TRACKING_TIME 20
#define SHORTEST_SAMPLE_SIZE 5
#define MIN_FPS 8.0
#define PATTERN_TYPES 9
#define PATTERN_TYPES_BEHAVIOR 3
#define DIRECTION_THRESHOLD 30.0
#define VIDEOTIME (30*60)
#define HOUGH_CIRCLE_RESIZE 2

//for SVM parameters
#define C_UPPER 15
#define C_LOWER -5

//for file name set up
#define MODEL_PATH_NAME "model/model_path.txt"

#endif // PARAMETERS


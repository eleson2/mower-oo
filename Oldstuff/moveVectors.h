// #include <globals.h>
// #include <driveunit.h>


     
// class moveVectors {
// private:
//     /* data */
// public:

//     static constexpr movement BWFLeft[] = {
//         { Speed40, Speed00, 400},
//         { Speed40, -Speed10, 200},
//         { Speed40, Speed10, 400},
//         { Speed40, Speed40, 600},
//         { Speed80, Speed80, 700},
//         { 0, 0, 0 }
//     };

//     static constexpr movement BWFRight[] = {
//         { Speed00, Speed40, 400},
//         { -Speed10, -Speed40, 200},
//         { Speed10, Speed40, 400},
//         { Speed40, Speed40, 600},
//         { Speed80, Speed80, 700},
//         { 0, 0, 0 }
//     };

//     static constexpr movement Circle[] = {
//         { Speed40, -Speed40, 800},
//         { -Speed10, -Speed40, 200},
//         { Speed70, Speed60, 14000},
//         { Speed90, Speed90, 14000},
//         { 0, 0, 0 }
//     };
  
//     moveVectors(/* args */) {};
//     ~moveVectors() {};

//     void Makemove(movement *moves) { 
//         int ls;
//         int rs;
//         int msec;

//         while (moves->mSec != 0 ) { 
//             ls = moves->leftSpeed;
//             ls = moves->rightSpeed;
//             ls = moves->mSec;
//             moves++;
//         }
//     };


//     movement *getMoves(CurrentMotion CM) {

//         movement * M;

//         switch (CM) {
//         case MOVE_STRAIGTH:
//             M = Circle;
//             break;
//         case CIRCLE:
//             M = Circle;
//             break;
//         case BWFLEFT:
//             M = BWFLeft;
//             break;
//         case BWFRIGHT:
//             M = BWFRight;
//             break;
//         default:
//             M = Circle;
//         };
//         return M;
//      };

// };


// // moveVectors::moveVectors(/* args */)
// // {
// // }

// // moveVectors::~moveVectors()
// // {
// // }

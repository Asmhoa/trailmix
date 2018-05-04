
// LINKED LIST TEST FOR APPEND
// TEST CASE:
// Making linkedList of Measure <-> Compute <-> Display <-> Annunciate <-> Status
//
// if(MeasureTask.prev == NULL) {
//     Serial.println("null <- measure");
// }
// if(MeasureTask.next == &ComputeTask) {
//     Serial.println("measure -> compute");
// }
// Serial.println("======================");
// if(ComputeTask.prev == &MeasureTask) {
//     Serial.println("measure <- compute");
// }
// if(ComputeTask.next == &DisplayTask) {
//     Serial.println("compute -> display");
// }
// Serial.println("======================");
// if(DisplayTask.prev == &ComputeTask) {
//     Serial.println("compute <- display");
// }
// if(DisplayTask.next == &AnnunciateTask) {
//     Serial.println("display -> annunciate");
// }
// Serial.println("======================");
// if(AnnunciateTask.prev == &DisplayTask) {
//     Serial.println("display <- annunciate");
// }
// if(AnnunciateTask.next == &StatusTask) {
//     Serial.println("annunciate -> status");
// }
// Serial.println("======================");
// if(StatusTask.prev == &AnnunciateTask) {
//     Serial.println("annunciate <- status");
// }
// if(StatusTask.next == NULL) {
//     Serial.println("status -> NULL");
// }
// Serial.println("======================");
// if(linkedListHead == &MeasureTask) {
//     Serial.println("linkedlisthead = measuretask");
// }

// LINKED LIST TEST FOR INSERT-AFTER
// TEST CASE:
// Making linkedList of  Measure <-> Compute <-> Display <-> Annunciate
//                       AND then we insert Status after Compute.
//                       Measure <-> Compute <-> Status <-> Display <-> Annunciate 
// appendAtEnd(&MeasureTask);
// appendAtEnd(&ComputeTask);
// appendAtEnd(&DisplayTask);
// appendAtEnd(&AnnunciateTask);
// insertAfterNode(&ComputeTask, &StatusTask);

// if(MeasureTask.prev == NULL) {
//     Serial.println("null <- measure");
// }
// if(MeasureTask.next == &ComputeTask) {
//     Serial.println("measure -> compute");
// }
// Serial.println("======================");
// if(ComputeTask.prev == &MeasureTask) {
//     Serial.println("measure <- compute");
// }
// if(ComputeTask.next == &StatusTask) {
//     Serial.println("compute -> status");
// }
// Serial.println("======================");
// if(StatusTask.prev == &ComputeTask) {
//     Serial.println("compute <- status");
// }
// if(StatusTask.next == &DisplayTask) {
//     Serial.println("status -> display");
// }
// Serial.println("======================");
// if(DisplayTask.prev == &StatusTask) {
//     Serial.println("status <- display");
// }
// if(DisplayTask.next == &AnnunciateTask) {
//     Serial.println("display -> annunciate");
// }
// Serial.println("======================");
// if(AnnunciateTask.prev == &DisplayTask) {
//     Serial.println("display <- annunciate");
// }
// if(AnnunciateTask.next == NULL) {
//     Serial.println("annunciate -> NULL");
// }
// Serial.println("======================");
// if(linkedListHead == &MeasureTask) {
//     Serial.println("linkedlisthead = measuretask");
// }


// LINKED LIST TEST FOR APPEND
// TEST CASE:
// Making linkedList of Measure <-> Compute <-> Display <-> Annunciate <-> Status
//
// appendAtEnd(&MeasureTask);
// appendAtEnd(&ComputeTask);
// appendAtEnd(&DisplayTask);
// appendAtEnd(&AnnunciateTask);
// appendAtEnd(&StatusTask);
// deleteNode(&StatusTask);

// if(MeasureTask.prev == NULL) {
//     Serial.println("null <- measure");
// }
// if(MeasureTask.next == &ComputeTask) {
//     Serial.println("measure -> compute");
// }
// Serial.println("======================");
// if(ComputeTask.prev == &MeasureTask) {
//     Serial.println("measure <- compute");
// }
// if(ComputeTask.next == &DisplayTask) {
//     Serial.println("compute -> display");
// }
// Serial.println("======================");
// if(DisplayTask.prev == &ComputeTask) {
//     Serial.println("compute <- display");
// }
// if(DisplayTask.next == &AnnunciateTask) {
//     Serial.println("display -> annunciate");
// }
// Serial.println("======================");
// if(AnnunciateTask.prev == &DisplayTask) {
//     Serial.println("display <- annunciate");
// }
// if(AnnunciateTask.next == NULL) {
//     Serial.println("annunciate -> NULL");
// }
// Serial.println("======================");
// if(linkedListHead == &MeasureTask) {
//     Serial.println("linkedlisthead = measuretask");
// }




/* SCHEDULE 
    TCB tasksArray[NUM_TASKS];
    tasksArray[0] = MeasureTask;
    tasksArray[1] = ComputeTask;
    tasksArray[2] = DisplayTask;
    tasksArray[3] = AnnunciateTask;
    tasksArray[4] = StatusTask;
    tasksArray[5] = NullTask;

    for(int i = 0; i < NUM_TASKS; i++) { // QUEUE
        tasksArray[i].taskFuncPtr(tasksArray[i].taskDataPtr);
    } */

    // MeasureTask.taskFuncPtr(MeasureTask.taskDataPtr);
    // //ComputeTask.taskFuncPtr(ComputeTask.taskDataPtr);
    
    // DisplayTask.taskFuncPtr(DisplayTask.taskDataPtr);
    // AnnunciateTask.taskFuncPtr(AnnunciateTask.taskDataPtr);
    // StatusTask.taskFuncPtr(StatusTask.taskDataPtr); 

    
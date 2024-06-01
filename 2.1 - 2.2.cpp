#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

using namespace std;

// 프로세스 클래스 정의
class Process {
public:
    int pid;  // 프로세스 ID
    bool is_fg;  // Foreground 프로세스 여부

    Process(int pid, bool is_fg = false) : pid(pid), is_fg(is_fg) {}
};

// 스택 노드 클래스 정의
class StackNode {
public:
    vector<Process*> process_list;  // 프로세스 리스트
    StackNode* next;  // 다음 스택 노드를 가리키는 포인터

    StackNode() : next(nullptr) {}
};

// Dynamic Queue 클래스 정의
class DynamicQueue {
    friend class MonitorProcess;
public:
    StackNode* get_top() const {
        return top;
    }
private:
    StackNode* top;  // 스택의 맨 위 노드를 가리키는 포인터
    int num_nodes;  // 스택 노드의 개수

public:
    DynamicQueue() : top(nullptr), num_nodes(0) {}

    // enqueue 함수: 스택의 맨 위 노드에 프로세스 추가
    void enqueue(Process* process) {
        if (top == nullptr) {
            top = new StackNode();
            num_nodes++;
        }
        if (process->is_fg) {
            // FG 프로세스면 리스트의 끝에 추가
            top->process_list.push_back(process);
        }
        else {
            // BG 프로세스면 리스트의 처음에 추가
            top->process_list.insert(top->process_list.begin(), process);
        }
    }

    // dequeue 함수: 스택의 맨 위 노드에서 프로세스 꺼내기
    Process* dequeue() {
        if (top == nullptr || top->process_list.empty()) {
            return nullptr;
        }
        Process* process = top->process_list.back();
        top->process_list.pop_back();
        if (top->process_list.empty()) {
            // 스택이 비어있으면 top 노드 제거
            StackNode* temp = top;
            top = top->next;
            delete temp;
            num_nodes--;
        }
        return process;
    }

    // promote 함수: 스택의 맨 위 노드에서 프로세스를 꺼내어 상위 리스트 노드의 꼬리에 붙이기
    void promote() {
        Process* process = dequeue();
        if (process == nullptr) {
            return;
        }
        if (top == nullptr) {
            top = new StackNode();
            num_nodes++;
        }
        top->process_list.push_back(process);
    }

    // split_n_merge 함수: 임계치(threshold)를 넘어설 경우 리스트의 앞쪽 절반을 상위 리스트 노드의 꼬리에 붙이기
    void split_n_merge(int threshold) {
        if (num_nodes > threshold) {
            StackNode* new_node = new StackNode();
            int half = num_nodes / 2;
            while (half--) {
                Process* process = dequeue();
                new_node->process_list.insert(new_node->process_list.begin(), process);
            }
            new_node->next = top;
            top = new_node;
            num_nodes++;
        }
    }

    // DQ(Dynamic Queue)와 WQ(Wait Queue) 상태 시각적으로 출력
    void print_dq_wq_status() {
        cout << "DQ: ";
        StackNode* current_node = top;
        while (current_node != nullptr) {
            for (Process* process : current_node->process_list) {
                cout << process->pid << " ";
            }
            current_node = current_node->next;
        }
        cout << endl;
    }
};

// Alarm Clock 구현
class AlarmClock {
private:
    int total_processes;  // 전체 프로세스 개수
    int num_nodes;  // 스택 노드의 개수
    int threshold;  // 임계치

public:
    AlarmClock(int total_processes, int num_nodes) : total_processes(total_processes), num_nodes(num_nodes), threshold(total_processes / num_nodes) {}

    // 새로운 임계치 설정
    void set_threshold(int new_threshold) {
        threshold = new_threshold;
    }

    // 현재 임계치 반환
    int get_threshold() {
        return threshold;
    }
};

// shell 프로세스 동작
class ShellProcess {
public:
    void execute_command(const string& command) {
        // 명령어 실행
    }

    void sleep(int seconds) {
        // 일정 시간 동안 sleep
        this_thread::sleep_for(chrono::seconds(seconds));
    }
};

// monitor 프로세스 동작
class MonitorProcess {
private:
    DynamicQueue& dynamic_queue;
    AlarmClock& alarm_clock;

public:
    MonitorProcess(DynamicQueue& dq, AlarmClock& ac) : dynamic_queue(dq), alarm_clock(ac) {}

    void print_dq_wq_status() {
        // DQ(Dynamic Queue)와 WQ(Wait Queue) 상태 시각적으로 출력
        cout << "DQ: ";
        StackNode* current_node = dynamic_queue.top;
        while (current_node != nullptr) {
            for (Process* process : current_node->process_list) {
                cout << process->pid << " ";
            }
            current_node = current_node->next;
        }
        cout << endl;
    }
};

int main() {
    // 초기화
    int total_processes = 20;  // 전체 프로세스 개수
    int num_nodes = 5;  // 스택 노드의 개수
    int sleep_time = 1;  // 스케줄러 호출 주기 (초 단위)

    DynamicQueue dynamic_queue;
    AlarmClock alarm_clock(total_processes, num_nodes);
    ShellProcess shell_process;
    MonitorProcess monitor_process(dynamic_queue, alarm_clock);

    // 프로세스 생성 및 enqueue
    for (int i = 0; i < total_processes; ++i) {
        Process* process = new Process(i, i % 2 == 0);  // 짝수 프로세스는 FG, 홀수 프로세스는 BG로 가정
        dynamic_queue.enqueue(process);
    }

    // 스케줄링 시작
    while (true) {
        // 스케줄러 호출
        monitor_process.print_dq_wq_status();
        alarm_clock.set_threshold(total_processes / num_nodes);  // 임계치 업데이트
        shell_process.sleep(sleep_time);
    }

    return 0;
}
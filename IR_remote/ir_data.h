#include <string.h>
#include <stddef.h>

#define NULL_IDX 255
unsigned char insert_name(unsigned char head, char *rem_name);
unsigned char insert_cmd_name(unsigned char head, char *cmd_name);
void insert_cmd(unsigned char head, unsigned char cmd, unsigned char idx);

struct IR_cmd {
	char cmd_name[10];
	unsigned char arr[4];
	unsigned char next;
	unsigned char prev;
};

struct IR {
	char name[10];
	unsigned char head_cmd_idx;
	unsigned char next;
	unsigned char prev;
};

struct IR xdata ir_pool[10];
struct IR_cmd xdata cmd_pool[100];
unsigned char cmd_idx = 0;
unsigned char ir_pool_idx = 0;

unsigned char insert_name(unsigned char head, char *rem_name){
	struct IR xdata *newnode; // declared at the top
	
	if(ir_pool_idx > 9){
		return head; // if mem full return 255
	}
	
	newnode = &ir_pool[ir_pool_idx++];
	strcpy(newnode->name, rem_name);
	newnode->next = head;
	newnode->prev = NULL_IDX;
	if(head != NULL_IDX){ // check if first remote
		struct IR xdata *temp;
		temp = &ir_pool[head];
		temp->prev = ir_pool_idx - 1;
	}
	newnode->head_cmd_idx = NULL_IDX;
	
	return (ir_pool_idx - 1);
}

unsigned char insert_cmd_name(unsigned char head, char *cmd_name){
	struct IR xdata *temp = &ir_pool[head];
	struct IR_cmd xdata *newcmd; // declared at the top
	
	if(head == NULL_IDX || cmd_idx > 99){
		return NULL_IDX; // if mem full return 255
	}
	
	newcmd = &cmd_pool[cmd_idx];
	newcmd->next = temp->head_cmd_idx;
	newcmd->prev = NULL_IDX;
	if(temp->head_cmd_idx != NULL_IDX){ //check if first cmd
		struct IR_cmd xdata *currcmd;
		currcmd = &cmd_pool[temp->head_cmd_idx];
		currcmd->prev = cmd_idx;
	}
	temp->head_cmd_idx = cmd_idx;
	cmd_idx++;
	
	strcpy(newcmd->cmd_name, cmd_name);
	
	return (cmd_idx - 1);
}

void insert_cmd(unsigned char head, unsigned char cmd, unsigned char idx){
	struct IR xdata *temp;
	struct IR_cmd xdata *temp_cmd;
	
	if(head == NULL_IDX || idx > 3){
		return;
	}
	
	temp = &ir_pool[head];
	if(temp->head_cmd_idx == NULL_IDX){
		return;
	}
	
	temp_cmd = &cmd_pool[temp->head_cmd_idx];
	temp_cmd->arr[idx] = cmd;
}

//next part written by a clanker

/* --- NAVIGATION & GETTER FUNCTIONS --- */

// Next Remote (stays on current if at the end of list)
unsigned char get_next_rem(unsigned char curr_rem) {
    if (curr_rem == NULL_IDX) return NULL_IDX;
    if (ir_pool[curr_rem].next != NULL_IDX) {
        return ir_pool[curr_rem].next;
    }
    return curr_rem;
}

// Previous Remote (stays on current if at the start of list)
unsigned char get_prev_rem(unsigned char curr_rem) {
    if (curr_rem == NULL_IDX) return NULL_IDX;
    if (ir_pool[curr_rem].prev != NULL_IDX) {
        return ir_pool[curr_rem].prev;
    }
    return curr_rem;
}

// Get first command index of the selected remote
unsigned char get_rem_cmd_head(unsigned char curr_rem) {
    if (curr_rem == NULL_IDX) return NULL_IDX;
    return ir_pool[curr_rem].head_cmd_idx;
}

// Next Command (stays on current if at the end of list)
unsigned char get_next_cmd(unsigned char curr_cmd) {
    if (curr_cmd == NULL_IDX) return NULL_IDX;
    if (cmd_pool[curr_cmd].next != NULL_IDX) {
        return cmd_pool[curr_cmd].next;
    }
    return curr_cmd;
}

// Previous Command (stays on current if at the start of list)
unsigned char get_prev_cmd(unsigned char curr_cmd) {
    if (curr_cmd == NULL_IDX) return NULL_IDX;
    if (cmd_pool[curr_cmd].prev != NULL_IDX) {
        return cmd_pool[curr_cmd].prev;
    }
    return curr_cmd;
}

// Return Remote Name string
char* get_rem_name(unsigned char curr_rem) {
    if (curr_rem == NULL_IDX) return "No Remote";
    return ir_pool[curr_rem].name;
}

// Return Command Name string
char* get_cmd_name(unsigned char curr_cmd) {
    if (curr_cmd == NULL_IDX) return "No Cmd";
    return cmd_pool[curr_cmd].cmd_name;
}

// Return a single byte (0-3) from a command's 4-byte IR array
unsigned char get_cmd_byte(unsigned char curr_cmd, unsigned char byte_idx) {
    if (curr_cmd == NULL_IDX || byte_idx > 3) return 0x00;
    return cmd_pool[curr_cmd].arr[byte_idx];
}

// Return pointer to the full 4-byte command array
unsigned char xdata* get_cmd_arr(unsigned char curr_cmd) {
    if (curr_cmd == NULL_IDX) return NULL;
    return cmd_pool[curr_cmd].arr;
}
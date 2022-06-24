/*
 *  linux/kernel/sys.c
 *
 *  (C) 1991  Linus Torvalds
 */

#include <errno.h>

#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/kernel.h>
#include <asm/segment.h>
#include <sys/times.h>
#include <sys/utsname.h>


int sys_ftime()
{
	return -ENOSYS;
}

int sys_break()
{
	return -ENOSYS;
}

int sys_ptrace()
{
	return -ENOSYS;
}

int sys_stty()
{
	return -ENOSYS;
}

int sys_gtty()
{
	return -ENOSYS;
}

int sys_rename()
{
	return -ENOSYS;
}

int sys_prof()
{
	return -ENOSYS;
}

int sys_setregid(int rgid, int egid)
{
	if (rgid>0) {
		if ((current->gid == rgid) || 
		    suser())
			current->gid = rgid;
		else
			return(-EPERM);
	}
	if (egid>0) {
		if ((current->gid == egid) ||
		    (current->egid == egid) ||
		    suser()) {
			current->egid = egid;
			current->sgid = egid;
		} else
			return(-EPERM);
	}
	return 0;
}

int sys_setgid(int gid)
{
/*	return(sys_setregid(gid, gid)); */
	if (suser())
		current->gid = current->egid = current->sgid = gid;
	else if ((gid == current->gid) || (gid == current->sgid))
		current->egid = gid;
	else
		return -EPERM;
	return 0;
}

int sys_acct()
{
	return -ENOSYS;
}

int sys_phys()
{
	return -ENOSYS;
}

int sys_lock()
{
	return -ENOSYS;
}

int sys_mpx()
{
	return -ENOSYS;
}

int sys_ulimit()
{
	return -ENOSYS;
}

int sys_time(long * tloc)
{
	int i;

	i = CURRENT_TIME;
	if (tloc) {
		verify_area(tloc,4);
		put_fs_long(i,(unsigned long *)tloc);
	}
	return i;
}

/*
 * Unprivileged users may change the real user id to the effective uid
 * or vice versa.
 */
int sys_setreuid(int ruid, int euid)
{
	int old_ruid = current->uid;
	
	if (ruid>0) {
		if ((current->euid==ruid) ||
                    (old_ruid == ruid) ||
		    suser())
			current->uid = ruid;
		else
			return(-EPERM);
	}
	if (euid>0) {
		if ((old_ruid == euid) ||
                    (current->euid == euid) ||
		    suser()) {
			current->euid = euid;
			current->suid = euid;
		} else {
			current->uid = old_ruid;
			return(-EPERM);
		}
	}
	return 0;
}

int sys_setuid(int uid)
{
/*	return(sys_setreuid(uid, uid)); */
	if (suser())
		current->uid = current->euid = current->suid = uid;
	else if ((uid == current->uid) || (uid == current->suid))
		current->euid = uid;
	else
		return -EPERM;
	return(0);
}

int sys_stime(long * tptr)
{
	if (!suser())
		return -EPERM;
	startup_time = get_fs_long((unsigned long *)tptr) - jiffies/HZ;
	return 0;
}

int sys_times(struct tms * tbuf)
{
	if (tbuf) {
		verify_area(tbuf,sizeof *tbuf);
		put_fs_long(current->utime,(unsigned long *)&tbuf->tms_utime);
		put_fs_long(current->stime,(unsigned long *)&tbuf->tms_stime);
		put_fs_long(current->cutime,(unsigned long *)&tbuf->tms_cutime);
		put_fs_long(current->cstime,(unsigned long *)&tbuf->tms_cstime);
	}
	return jiffies;
}

int sys_brk(unsigned long end_data_seg)
{
	if (end_data_seg >= current->end_code &&
	    end_data_seg < current->start_stack - 16384)
		current->brk = end_data_seg;
	return current->brk;
}

/*
 * This needs some heave checking ...
 * I just haven't get the stomach for it. I also don't fully
 * understand sessions/pgrp etc. Let somebody who does explain it.
 */
int sys_setpgid(int pid, int pgid)
{
	int i;

	if (!pid)
		pid = current->pid;
	if (!pgid)
		pgid = current->pid;
	for (i=0 ; i<NR_TASKS ; i++)
		if (task[i] && task[i]->pid==pid) {
			if (task[i]->leader)
				return -EPERM;
			if (task[i]->session != current->session)
				return -EPERM;
			task[i]->pgrp = pgid;
			return 0;
		}
	return -ESRCH;
}

int sys_getpgrp(void)
{
	return current->pgrp;
}

int sys_setsid(void)
{
	if (current->leader && !suser())
		return -EPERM;
	current->leader = 1;
	current->session = current->pgrp = current->pid;
	current->tty = -1;
	return current->pgrp;
}

int sys_getgroups()
{
	return -ENOSYS;
}

int sys_setgroups()
{
	return -ENOSYS;
}

int sys_uname(struct utsname * name)
{
	static struct utsname thisname = {
		"linux .0","nodename","release ","version ","machine "
	};
	int i;

	if (!name) return -ERROR;
	verify_area(name,sizeof *name);
	for(i=0;i<sizeof *name;i++)
		put_fs_byte(((char *) &thisname)[i],i+(char *) name);
	return 0;
}

int sys_sethostname()
{
	return -ENOSYS;
}

int sys_getrlimit()
{
	return -ENOSYS;
}

int sys_setrlimit()
{
	return -ENOSYS;
}

int sys_getrusage()
{
	return -ENOSYS;
}

int sys_gettimeofday()
{
	return -ENOSYS;
}

int sys_settimeofday()
{
	return -ENOSYS;
}


int sys_umask(int mask)
{
	int old = current->umask;

	current->umask = mask & 0777;
	return (old);
}


int sys_pipe2(int *pipedf, int flags)
{
	struct m_inode *inode;
	struct file *f[2];
	int fd[2];
	int i, j;

	j = 0;
	for (i = 0; j < 2 && i < NR_FILE; i++)
		if (!file_table[i].f_count)
			(f[j++] = i + file_table)->f_count++;
	if (j == 1)
		f[0]->f_count = 0;
	if (j < 2)
		return -1;
	j = 0;
	for (i = 0; j < 2 && i < NR_OPEN; i++)
		if (!current->filp[i])
		{
			current->filp[fd[j] = i] = f[j];
			j++;
		}
	if (j == 1)
		current->filp[fd[0]] = NULL;
	if (j < 2)
	{
		f[0]->f_count = f[1]->f_count = 0;
		return -1;
	}
	if (!(inode = get_pipe_inode()))
	{
		current->filp[fd[0]] =
			current->filp[fd[1]] = NULL;
		f[0]->f_count = f[1]->f_count = 0;
		return -1;
	}
	f[0]->f_inode = f[1]->f_inode = inode;
	f[0]->f_pos = f[1]->f_pos = 0;
	f[0]->f_mode = 1; /* read */
	f[1]->f_mode = 2; /* write */
	put_fs_long(fd[0], 0 + pipedf);
	put_fs_long(fd[1], 1 + pipedf);
	return 0;
}

long sys_getcwd(char *buf,size_t size){
	int entries;
	int block,i;
	int num = 19;
	char s[20][NAME_LEN];
	struct buffer_head * bh;
	struct dir_entry * de;
	struct super_block * sb;
	struct m_inode * dir =current->pwd;
	struct m_inode * olddir  =NULL;
	while(1){
		olddir = dir;
		if (olddir==current->root)
			break;
		if (!dir->i_zone[0])
			return -1;
		if(!(bh = bread(dir->i_dev,dir->i_zone[0])))//获得i节点指向的数据块
			return -1;
		de = (struct dir_entry *) bh->b_data;
		int find = de->inode;
		if (!(dir = iget(dir->i_dev,(de+1)->inode))){//dir更新并判断错误
			return -1;
		}
		if(!(bh = bread(dir->i_dev,dir->i_zone[0])))//获得i节点指向的数据块
			return -1;
		de = (struct dir_entry *) bh->b_data;
		//printk("dir_size is %d \n",dir->i_size);
		int flag = 1;
		int f1=1,f2=1;
		//printk("find inode is %d\n",find);
		while(flag<=(dir->i_size/16)){
		//	printk("de->i_node is %d de->name is %s \n",de->inode,de->name);
			if(de->inode==find){
				//printk("%s\n",de->name);
				strcpy(s[num--],de->name);
				//printk(" file is  %s \n",s[num+1]);
				break;
			}
			f1++;
			if (f1>1024/16){
				bh = bread(dir->i_dev,dir->i_zone[f2++]);
				de = (struct dir_entry *) bh->b_data;
			}
			de++;
			flag++;
		}
	}
	size_t len = 0;
	char *buf1;
	for(i=num+1;i<=19;i++){
		printk("/%s",s[i]);
	}
	printk("\n");
	//printk("getcwd is %s \n",buf1);
	//return NULL;
}




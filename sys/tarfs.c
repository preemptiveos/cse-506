#include <defs.h>
#include <print.h>
#include <sys/tarfs.h>
#include <common.h>
#include <stdio.h>
#include <string.h>

tarfs_entry tarfs_fs[100];

int get_per_ind(char* dir)
{
	char name[100];
    	int len = strlen(dir);
    	strcpy(&name[0], dir);
    	len = len-2;
   	// kkprintff("  {%d} ", len); 
    	while(name[len] != '/')
    	{
        	len--;
        	if(len == 0)
            		return 999;
    	}
   	// kkprintff("  {%d} ", len); 
    	name[++len] = '\0';
    	int i = 0;
    	while(strcmp(&name[0], &(tarfs_fs[i].name[0])) !=  0)
        	i++;
    	// kkprintff("parent {%d}", i);
    	return i;
}

int get_per_ind_file(char* dir)
{
    	char name[100];
    	int len = strlen(dir);
    	strcpy(&name[0], dir);
    	len = len-1;
   	// kkprintff("  {%d} ", len); 
    	while(name[len] != '/')
    	{
        	len--;
        	if(len == 0)
            		return 999;
    	}
   	// kkprintff("  {%d} ", len); 
    	name[++len] = '\0';
    	int i = 0;
    	while(strcmp(&name[0], &(tarfs_fs[i].name[0])) !=  0)
        	i++;
    	// kkprintff("parent {%d}", i);
    	return i;
}

uint64_t is_file_exists(char* filename)
{
	// kkprintff("\n Binary tarfs start:  %x", &_binary_tarfs_start);
	struct posix_header_ustar* test_tarfs = (struct posix_header_ustar *)&_binary_tarfs_start;
	int i = 1, temp = 512;
	uint64_t size;
	// kkprintff("\n Name %s \t Mode %s \t uid %s \t gid %s\t",test_tarfs->name, test_tarfs->mode, test_tarfs->uid, test_tarfs->gid);
	while(strlen(test_tarfs->name) != 0)
	{
		test_tarfs = (struct posix_header_ustar *)(&_binary_tarfs_start + temp);
		size = octalToDecimal(stoi(test_tarfs->size));
		// kkprintff("\nName of file is %s and size in octal is %d", test_tarfs->name, test_tarfs->size);
		if(strlen(test_tarfs->name) == 0)
			return 999;
		if(strcmp(test_tarfs->name, filename) >= 0)
			return temp + 512;
		if(size == 0)
			temp = temp + 512;
		else
			temp +=  (size%512==0) ? size + 512 : size + (512 - size%512) + 512;
		// kkprintff("    %d", temp);
		i += 1;
	}
	// kkprintff("%d", size);
	return 0;
}

/*
void get_file_sections(char* filename)
{
	uint64_t offset = is_file_exists(filename);
	// kkprintff("\n %d File Offset", offset);
	//char *file_start= (char *)(&_binary_tarfs_start + offset);
	struct posix_header_ustar *test_tarfs = (struct posix_header_ustar *)(&_binary_tarfs_start + offset - 512);
	// kkprintff("\n Magic number %s",test_tarfs->magic);
	// kkprintff("\n Name %s \t Mode %s \t uid %s \t gid %s\t",test_tarfs->name, test_tarfs->mode, test_tarfs->uid, test_tarfs->gid);
	Elf64_Ehdr *elfhdr = (Elf64_Ehdr *) (&_binary_tarfs_start + offset);
	kkprintff("\nentry %x", elfhdr->e_entry);
	kkprintff("\nphysoffset %x num %d", elfhdr->e_phoff, elfhdr->e_phnum);
	kkprintff("\nssoffset %x num %d", elfhdr->e_shoff, elfhdr->e_shnum);
	Elf_hdr *elfhdr1 = ( Elf_hdr *) (&_binary_tarfs_start + offset);
		/// kkprintff("\n %x", elfhdr1->e_entry);
	// kkprintff("\nphysoffset %x num %d", elfhdr1->e_phoff, elfhdr1->e_phnum);
	// kkprintff("\nssoffset %x num %d", elfhdr1->e_shoff, elfhdr1->e_shnum);
	
	Elf64_Phdr *elfhdr_p = (Elf64_Phdr *)((uint64_t)elfhdr1 + (uint64_t)elfhdr1->e_phoff);
	// kkprintff("\nphysoffset %x ", elfhdr_p->p_paddr);
	//Elf64_Shdr *elfhdr_s = (Elf64_Shdr *)((uint64_t)elfhdr1 + (uint64_t)elfhdr1->e_shoff);
//	int i = 0;
	
	//for(i=0; i<16; i++)
	//{
	
		kkprintff("\noffset %x ", elfhdr_s->sh_offset);
		kkprintff("\naddress %x ", elfhdr_s->sh_addr);
		kkprintff("  type %x ", elfhdr_s->sh_type);
		kkprintff("  size %x ", elfhdr_s->sh_size);
		kkprintff("  name %c ", elfhdr_s->sh_name);
	//	elfhdr_s++;
	//}
	
	//puts((char *)*elfhdr_s->sh_name);
	struct Proghdr *ph, *eph;
	if (elfhdr->e_magic != ELF_MAGIC)
		kkprintff("elf header's magic is not correct\n");
	kkprintff("\n %x", elfhdr);
	kkprintff("\n %x", elfhdr->e_phoff);
	kkprintff("\n %x", elfhdr->e_phnum);
	kkprintff("\n %x", elfhdr->e_shnum);
	ph = (struct Proghdr *) ((uint8_t *) elfhdr + elfhdr->e_phoff);
	kkprintff("\n %x", ph->p_filesz);
	kkprintff("\n %x", ph->p_offset);
	kkprintff("\n %x", elfhdr->e_shnum);

	eph = ph + elfhdr->e_phnum;
	kkprintff("\n %x", eph->p_filesz);
	kkprintff("\n %x", ph->p_offset);
	struct Secthdr *sectHdr = (struct Secthdr *)elfhdr + elfhdr->e_shoff;
	kkprintff("\n %x", elfhdr->e_shoff);
	kkprintff("\n %x", sectHdr->sh_offset);*/
	//puts((char *)(sectHdr->sh_name));
	/*lcr3(PADDR(e->env_pgdir));*/

	/*int i;
	for(i=0; i< 1000; i++)
	{
		kkprintff("%c", *file_start++);
	}
}
*/
void tarfs_init()
{
       	struct posix_header_ustar *tarfs_var = (struct posix_header_ustar *)&_binary_tarfs_start;
	int i = 0, temp = 0;
	uint64_t size;
       	tarfs_entry tarfs_e;
       	//int curr_dir= 999;
	while(true)
	{
		tarfs_var = (struct posix_header_ustar *)(&_binary_tarfs_start + temp);
           	if(strlen(tarfs_var->name) == 0)
                	break;
          
		size = octalToDecimal(stoi(tarfs_var->size));
           	strcpy(&tarfs_e.name[0], tarfs_var->name);
           	tarfs_e.size = size;
           	tarfs_e.addr_hdr = (uint64_t)&_binary_tarfs_start + temp;
           	tarfs_e.typeflag = stoi(tarfs_var->typeflag);
           	if(tarfs_e.typeflag == FILE_TYPE)
                	tarfs_e.par_ind = get_per_ind_file(&(tarfs_e.name[0]));
           	else if(tarfs_e.typeflag == DIRECTORY)
           	{
                	tarfs_e.par_ind = get_per_ind(&(tarfs_e.name[0]));
               		// curr_dir = i;
           	}
           
            
           	tarfs_fs[i] = tarfs_e;
           	// kkprintff("%p", &(tarfs_fs[i].name[0])); 
           	// kkprintff("   I[%d]     P[%d] \n", i, tarfs_fs[i].par_ind);
           	i++;
		if(size == 0)
			temp = temp + 512;
		else
			temp +=  (size%512==0) ? size + 512 : size + (512 - size%512) + 512;
	  }
        
      	// read_dir("lib/");
      	// kkprintff("\n%x", open("bin/hello"));
      	// char test[10];
      	// read_file(open("aaatest"), 4, (uint64_t)test);
}

uint64_t open_dir(char * dir)
{
	tarfs_entry tarfs_e;
        int i = 0;
        while(true )
        {
            tarfs_e = tarfs_fs[i];
            i++;
            if(strlen(tarfs_e.name) == 0)
                    break;

            if(strcmp(&(tarfs_e.name[0]), dir) == 0 && tarfs_e.typeflag == DIRECTORY)
                    return tarfs_e.addr_hdr;
        }
        kprintf("\n No such directory ");
        kprintf("%s", dir);
        return 0;
}

uint64_t read_dir(char* dir)
{
	tarfs_entry tarfs_e;
    	int i = 0, parent = -1;
    	while(true )
    	{
        	tarfs_e = tarfs_fs[i];
        	if(strlen(tarfs_e.name) == 0)
            		break;
        
        	if( strcmp(&(tarfs_e.name[0]), dir) == 0)
         	{  
            		parent = i;
			i++; 
			continue; 
         	}
         	if((strncmp(&(tarfs_e.name[0]), dir, strlen(dir)) == 0 ) &&( tarfs_e.par_ind == parent))
         	{
            		kprintf("%s", tarfs_e.name + strlen(dir));
			kprintf("\n");
         	}
        	i++;
    	}
    	return 0;
}

uint64_t open(char * file)
{
	tarfs_entry tarfs_e;
        int i = 0;
        while(true )
        {
            tarfs_e = tarfs_fs[i];
            i++;
            if(strlen(tarfs_e.name) == 0)
                    break;

            if(strcmp(&(tarfs_e.name[0]), file) == 0 && tarfs_e.typeflag == FILE_TYPE)
                    return tarfs_e.addr_hdr;
        }
        kprintf("\n No such file ");
        kprintf("%s", file);
        return 0;
}

int read_file(uint64_t file_addr, int size, uint64_t buf)
{
   	struct posix_header_ustar* file_hdr = (struct posix_header_ustar *) file_addr; 
    	// kprintf("%s", file_hdr->name);
    	int file_size =  octalToDecimal(stoi(file_hdr->size));
    	if(file_size < size)
        	size = file_size;
    	char* tmp =(char *)buf;
    	char* file_start_addr = (char *)(file_addr + 512);
    	// kprintf("\nsize %d file %x", size, file_start_addr);
    	int i = 0;
    	while(size-- > 0)
    	{
        	tmp[i++] = *file_start_addr++;
    	}    
    	tmp[i]='\0';
    //	kprintf("\n");
    //	kprintf("%s", tmp);
    	return size;
}

void close(uint64_t file_addr)
//	kprintf("Inside close file");
{

	unsigned long* file = (unsigned long* )file_addr;
	*file = 0;
	kprintf("\nFile at %p has been closed",file_addr);
}

void closedir(uint64_t dir_addr)
//      kprintf("Inside close file");
{

        unsigned long* dir = (unsigned long* )dir_addr;
        *dir = 0;
        kprintf("\nDirectory at %p has been closed",dir_addr);
}


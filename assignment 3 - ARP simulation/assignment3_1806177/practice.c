// convert to ip[7] format
void extract_ip(char *ip_to, char *ip_from)
{
	char temp[5];
	strncpy(temp, ip_from, 3);
	temp[4] = '\0';
	ip_to[0] = atoi(temp);
	strncpy(temp, ip_from + 4, 3);
	temp[4] = '\0';
	ip_to[1] = atoi(temp);
	strncpy(temp, ip_from + 8, 1);
	temp[1] = '\0';
	ip_to[2] = atoi(temp);
	strncpy(temp, ip_from + 10, 2);
	temp[2] = '\0';
	ip_to[3] = atoi(temp);

	ip_to[4] = '\0';
}


void print_ip(char *ip)
{
	int i;
	for (i = 0; i < 4; i++)
	{
		printf("%d", (unsigned char)ip[i]);
		if (i != 3)
			printf(".");
	}
}

int main(){
    char ip_to[5];
    char ip_from[] = "192.168.4.20";

    extract_ip(ip_to, ip_from);
    print_ip(ip_to);
}

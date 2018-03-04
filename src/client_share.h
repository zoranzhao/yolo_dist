#include "darknet_dist_mr.h"
void send_result_share(dataBlob* blob, const char *dest_ip, int portno);

inline int bind_port_client();

void get_data_and_send_result_to_gateway(unsigned int number_of_jobs, int sockfd, std::string thread_name){
    int newsockfd;
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    clilen = sizeof(cli_addr);

    for(int frame = 0; frame < number_of_jobs; frame++){
	    unsigned int total_part_num = 0;
	    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	    if (newsockfd < 0) sock_error("ERROR on accept");
	    read_sock(newsockfd, (char*)&total_part_num, sizeof(total_part_num));
	    std::cout << "Recved task number is: "<< total_part_num << std::endl;
	    close(newsockfd);


	    int job_id; 
	    unsigned int bytes_length;  
	    char* blob_buffer;
	    for(int i = 0; i < total_part_num; i++){
	       newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	       read_sock(newsockfd, (char*)&job_id, sizeof(job_id));
	       read_sock(newsockfd, (char*)&bytes_length, sizeof(bytes_length));
	       blob_buffer = (char*)malloc(bytes_length);
	       read_sock(newsockfd, blob_buffer, bytes_length);
	       std::cout << "Recved task : "<< job_id << " Size is: "<< bytes_length << std::endl;
	       put_job(blob_buffer, bytes_length, job_id);
	       close(newsockfd);
	    }

	    for(int i = 0; i < total_part_num; i++){
		dataBlob* blob = result_queue.Dequeue();
		send_result(blob, AP, PORTNO);
	    }
    }
}



inline void forward_network_dist_share(network *netp, int sockfd)
{
    int newsockfd;
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    clilen = sizeof(cli_addr);

    network net = *netp;

    int startfrom = 0;
    int upto = STAGES-1;
    if(netp -> input != NULL ){

      char request_type[10];
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
      read_sock(newsockfd, request_type, 10);

      int cli_id = 0;//TODO
      dataBlob* blob = new dataBlob(netp -> input, (stage_input_range.w)*(stage_input_range.h)*(net.layers[0].c)*sizeof(float), cli_id); 
      std::cout << "Sending the entire input to gateway ..." << std::endl;
      send_result_share(blob, AP, PORTNO);
      //free(netp -> input);
      delete blob;
    }

    float* data;
    int part_id;
    unsigned int size;

    for(int part = 0; 1; part ++){
       try_get_job((void**)&data, &size, &part_id);
       if(data == NULL) {
	   break;
       }
       std::cout<< "Processing task "<< part_id <<std::endl;
       net = forward_stage(part_id/PARTITIONS_W, part_id%PARTITIONS_W,  data, startfrom, upto, net);
       put_result(net.layers[upto].output, net.layers[upto].outputs* sizeof(float), part_id);
       free(data);
    }

}


void client_with_image_input_share(network *netp, unsigned int number_of_jobs, int sockfd, std::string thread_name)
{
    network *net = netp;
    srand(2222222);
#ifdef NNPACK
    nnp_initialize();
    net->threadpool = pthreadpool_create(THREAD_NUM);
#endif

    int id = 0;//5000 > id > 0
    unsigned int cnt = 0;//5000 > id > 0

    for(cnt = 0; cnt < number_of_jobs; cnt ++){
        image sized;
	sized.w = net->w; sized.h = net->h; sized.c = net->c;
	id = cnt;
        load_image_by_number(&sized, id);
        net->input  = sized.data;
        net->truth = 0;
        net->train = 0;
        net->delta = 0;
        forward_network_dist_share(net, sockfd);
        free_image(sized);
    }
#ifdef NNPACK
    pthreadpool_destroy(net->threadpool);
    nnp_deinitialize();
#endif
}

void client_without_image_input_share(network *netp, unsigned int number_of_jobs, int sockfd, std::string thread_name)
{
    network *net = netp;
    srand(2222222);
#ifdef NNPACK
    nnp_initialize();
    net->threadpool = pthreadpool_create(THREAD_NUM);
#endif

    for(int cnt = 0; cnt < number_of_jobs; cnt ++){
        net->input  = NULL;
        net->truth = 0;
        net->train = 0;
        net->delta = 0;
        forward_network_dist_share(net, sockfd);
    }
#ifdef NNPACK
    pthreadpool_destroy(net->threadpool);
    nnp_deinitialize();
#endif
}




void busy_client_share(){
    unsigned int number_of_jobs = 4;
    network *netp = load_network((char*)"cfg/yolo.cfg", (char*)"yolo.weights", 0);
    set_batch_network(netp, 1);
    network net = reshape_network(0, STAGES-1, *netp);
    exec_control(START_CTRL);
    int sockfd = bind_port_client();
    g_t1 = 0;
    g_t0 = what_time_is_it_now();
    std::thread t1(client_with_image_input_share, &net, number_of_jobs, sockfd, "client_with_image_input_share");
    std::thread t2(get_data_and_send_result_to_gateway, number_of_jobs, sockfd, "get_data_and_send_result_to_gateway");
    t1.join();
    t2.join();
}


void idle_client_share(){
    unsigned int number_of_jobs = 4;
    network *netp = load_network((char*)"cfg/yolo.cfg", (char*)"yolo.weights", 0);
    set_batch_network(netp, 1);
    network net = reshape_network(0, STAGES-1, *netp);
    exec_control(START_CTRL);
    int sockfd = bind_port_client();
    g_t1 = 0;
    g_t0 = what_time_is_it_now();
    std::thread t1(client_without_image_input_share, &net, number_of_jobs, sockfd, "client_without_image_input_share");
    std::thread t2(get_data_and_send_result_to_gateway, number_of_jobs, sockfd, "get_data_and_send_result_to_gateway");
    t1.join();
    t2.join();
}




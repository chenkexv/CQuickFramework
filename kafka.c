/**
 *  Copyright 2015 Elias Van Ootegem.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * Special thanks to Patrick Reilly and Aleksandar Babic for their work
 * On which this extension was actually built.
 */
#include <php.h>
#include "kafka.h"
#include <php_CKafka.h>
#include <inttypes.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>
#include "kafka.h"
#include "librdkafka/rdkafka.h"

struct consume_cb_params {
    int read_count;
    zval *return_value;
    union {
        int *partition_ends;
        long *partition_offset;
    };
    int error_count;
    int eop;
    int auto_commit;
};

struct produce_cb_params {
    int msg_count;
    int err_count;
    int offset;
    int partition;
    int errmsg_len;
    char *err_msg;
};

static int log_level = 1;
static rd_kafka_t *rk = NULL;
static rd_kafka_type_t rk_type;
char *brokers = "localhost:9092";
int partition = RD_KAFKA_PARTITION_UA;

void kafka_connect(char *brokers)
{
    kafka_setup(brokers);
}

void kafka_set_log_level( int ll )
{
    log_level = ll;
}

void kafka_msg_delivered (rd_kafka_t *rk,
                           void *payload, size_t len,
                           int error_code,
                           void *opaque, void *msg_opaque)
{
    if (error_code && log_level) {
        openlog("phpkafka", 0, LOG_USER);
        syslog(LOG_INFO, "phpkafka - Message delivery failed: %s",
                rd_kafka_err2str(error_code));
    }
}

void kafka_err_cb (rd_kafka_t *rk, int err, const char *reason, void *opaque)
{
    if (log_level) {
        openlog("phpkafka", 0, LOG_USER);
        syslog(LOG_INFO, "phpkafka - ERROR CALLBACK: %s: %s: %s\n",
            rd_kafka_name(rk), rd_kafka_err2str(err), reason);
    }
    if (rk)
        rd_kafka_destroy(rk);
}

void kafka_produce_cb_simple(rd_kafka_t *rk, void *payload, size_t len, int err_code, void *opaque, void *msg_opaque)
{
    struct produce_cb_params *params = msg_opaque;
    if (params)
    {
        params->msg_count -=1;
    }
    if (log_level)
    {
        if (params)
            params->err_count += 1;
        openlog("phpkafka", 0, LOG_USER);
        if (err_code)
            syslog(LOG_ERR, "Failed to deliver message %s: %s", (char *) payload, rd_kafka_err2str(err_code));
        else
            syslog(LOG_DEBUG, "Successfuly delevired message (%zd bytes)", len);
    }
}

void kafka_produce_detailed_cb(rd_kafka_t *rk, const rd_kafka_message_t *msg, void *opaque)
{
    struct produce_cb_params *params = opaque;
    if (params)
    {
        params->msg_count -= 1;
    }
    if (msg->err)
    {
        int offset = params->errmsg_len,
            err_len = 0;
        const char *errstr = rd_kafka_message_errstr(msg);
        err_len = strlen(errstr);
        if (log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(LOG_ERR, "Failed to deliver message: %s", errstr);
        }
        if (params)
        {
            params->err_count += 1;
            params->err_msg = realloc(
                params->err_msg,
                (offset + err_len + 2) * sizeof params->err_msg
            );
            if (params->err_msg == NULL)
            {
                params->errmsg_len = 0;
            }
            else
            {
                strcpy(
                    params->err_msg + offset,
                    errstr
                );
                offset += err_len;//get new strlen
                params->err_msg[offset] = '\n';//add new line
                ++offset;
                params->err_msg[offset] = '\0';//ensure zero terminated string
            }
        }
        return;
    }
    if (params)
    {
        params->offset = msg->offset;
        params->partition = msg->partition;
    }
}

rd_kafka_t *kafka_get_connection(kafka_connection_params params, const char *brokers)
{
    rd_kafka_t *r = NULL;
    char errstr[512];
    rd_kafka_conf_t *conf = rd_kafka_conf_new();
    //set error callback
    rd_kafka_conf_set_error_cb(conf, kafka_err_cb);
    if (params.type == RD_KAFKA_CONSUMER)
    {


        if (params.queue_buffer){
            rd_kafka_conf_set(conf, "queued.min.messages", params.queue_buffer, NULL, 0);
		}

		//创建连接
        r = rd_kafka_new(params.type, conf, errstr, sizeof errstr);


        if (!r)
        {
            if (params.log_level)
            {
                openlog("phpkafka", 0, LOG_USER);
                syslog(LOG_ERR, "Failed to connect to kafka: %s", errstr);
            }
            //destroy config, no connection to use it...
            rd_kafka_conf_destroy(conf);
            return NULL;
        }
        if (!rd_kafka_brokers_add(r, brokers))
        {
            if (params.log_level)
            {
                openlog("phpkafka", 0, LOG_USER);
                syslog(LOG_ERR, "Failed to connect to brokers %s", brokers);
            }
            rd_kafka_destroy(r);
            return NULL;
        }


        return r;
    }
    if (params.compression)
    {
        rd_kafka_conf_res_t result = rd_kafka_conf_set(
            conf, "compression.codec",params.compression, errstr, sizeof errstr
        );
        if (result != RD_KAFKA_CONF_OK)
        {
            if (params.log_level)
            {
                openlog("phpkafka", 0, LOG_USER);
                syslog(LOG_ALERT, "Failed to set compression %s: %s", params.compression, errstr);
            }
            rd_kafka_conf_destroy(conf);
            return NULL;
        }
    }
    if (params.retry_count)
    {
        rd_kafka_conf_res_t result = rd_kafka_conf_set(
            conf, "message.send.max.retries",params.retry_count, errstr, sizeof errstr
        );
        if (result != RD_KAFKA_CONF_OK)
        {
            if (params.log_level)
            {
                openlog("phpkafka", 0, LOG_USER);
                syslog(LOG_ALERT, "Failed to set compression %s: %s", params.compression, errstr);
            }
            rd_kafka_conf_destroy(conf);
            return NULL;
        }
    }
    if (params.retry_interval)
    {
        rd_kafka_conf_res_t result = rd_kafka_conf_set(
            conf, "retry.backoff.ms",params.retry_interval, errstr, sizeof errstr
        );
        if (result != RD_KAFKA_CONF_OK)
        {
            if (params.log_level)
            {
                openlog("phpkafka", 0, LOG_USER);
                syslog(LOG_ALERT, "Failed to set compression %s: %s", params.compression, errstr);
            }
            rd_kafka_conf_destroy(conf);
            return NULL;
        }
    }
    if (params.reporting == 1)
        rd_kafka_conf_set_dr_cb(conf, kafka_produce_cb_simple);
    else if (params.reporting == 2)
        rd_kafka_conf_set_dr_msg_cb(conf, kafka_produce_detailed_cb);
    r = rd_kafka_new(params.type, conf, errstr, sizeof errstr);
    if (!r)
    {
        if (params.log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(LOG_ERR, "Failed to connect to kafka: %s", errstr);
        }
        //destroy config, no connection to use it...
        rd_kafka_conf_destroy(conf);
        return NULL;
    }
    if (!rd_kafka_brokers_add(r, brokers))
    {
        if (params.log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(LOG_ERR, "Failed to connect to brokers %s", brokers);
        }
        rd_kafka_destroy(r);
        return NULL;
    }
    return r;
}

rd_kafka_t *kafka_set_connection(rd_kafka_type_t type, const char *b, int report_level, const char *compression)
{
    rd_kafka_t *r = NULL;
    char *tmp = brokers;
    char errstr[512];
    rd_kafka_conf_t *conf = rd_kafka_conf_new();
    if (!(r = rd_kafka_new(type, conf, errstr, sizeof(errstr)))) {
        if (log_level) {
            openlog("phpkafka", 0, LOG_USER);
            syslog(LOG_INFO, "phpkafka - failed to create new producer: %s", errstr);
        }
        exit(1);
    }
    /* Add brokers */
    if (rd_kafka_brokers_add(r, b) == 0) {
        if (log_level) {
            openlog("phpkafka", 0, LOG_USER);
            syslog(LOG_INFO, "php kafka - No valid brokers specified");
        }
        exit(1);
    }
    /* Set up a message delivery report callback.
     * It will be called once for each message, either on successful
     * delivery to broker, or upon failure to deliver to broker. */
    if (type == RD_KAFKA_PRODUCER)
    {
        if (compression && !strcmp(compression, "none"))
        {//silently fail on error ATM...
            if (RD_KAFKA_CONF_OK != rd_kafka_conf_set(conf, "compression.codec", compression, errstr, sizeof errstr))
            {
                if (log_level)
                {
                    openlog("phpkafka", 0, LOG_USER);
                    syslog(LOG_INFO, "Failed to set compression to %s", compression);
                }
            }
        }
        if (report_level == 1)
            rd_kafka_conf_set_dr_cb(conf, kafka_produce_cb_simple);
        else if (report_level == 2)
            rd_kafka_conf_set_dr_msg_cb(conf, kafka_produce_detailed_cb);
    }
    rd_kafka_conf_set_error_cb(conf, kafka_err_cb);

    if (log_level) {
        openlog("phpkafka", 0, LOG_USER);
        syslog(LOG_INFO, "phpkafka - using: %s", brokers);
    }
    return r;
}

void kafka_set_partition(int partition_selected)
{
    partition = partition_selected;
}

void kafka_setup(char* brokers_list)
{
    brokers = brokers_list;
}

void kafka_destroy(rd_kafka_t *r, int timeout)
{
    if(r != NULL)
    {
        //poll handle status
        rd_kafka_poll(r, 0);
        if (rd_kafka_outq_len(r) > 0)
        {//wait for out-queue to clear
            while(rd_kafka_outq_len(r) > 0)
                rd_kafka_poll(r, timeout);
            timeout = 1;
        }
        rd_kafka_destroy(r);
        //this wait is blocking PHP
        //not calling it will yield segfault, though
        rd_kafka_wait_destroyed(timeout);
        r = NULL;
    }
}

//We're no longer relying on the global rk variable (not thread-safe)
static void kafka_init( rd_kafka_type_t type )
{
    if (rk && type != rk_type)
    {
        rd_kafka_destroy(rk);
        rk = NULL;
    }
    if (rk == NULL)
    {
        char errstr[512];
        rd_kafka_conf_t *conf = rd_kafka_conf_new();
        if (!(rk = rd_kafka_new(type, conf, errstr, sizeof(errstr)))) {
            if (log_level) {
                openlog("phpkafka", 0, LOG_USER);
                syslog(LOG_INFO, "phpkafka - failed to create new producer: %s", errstr);
            }
            exit(1);
        }
        /* Add brokers */
        if (rd_kafka_brokers_add(rk, brokers) == 0) {
            if (log_level) {
                openlog("phpkafka", 0, LOG_USER);
                syslog(LOG_INFO, "php kafka - No valid brokers specified");
            }
            exit(1);
        }
        /* Set up a message delivery report callback.
         * It will be called once for each message, either on successful
         * delivery to broker, or upon failure to deliver to broker. */
        if (type == RD_KAFKA_PRODUCER)
            rd_kafka_conf_set_dr_cb(conf, kafka_produce_cb_simple);
        rd_kafka_conf_set_error_cb(conf, kafka_err_cb);

        if (log_level) {
            openlog("phpkafka", 0, LOG_USER);
            syslog(LOG_INFO, "phpkafka - using: %s", brokers);
        }
    }
}

int kafka_produce_report(rd_kafka_t *r, const char *topic, char *msg, int msg_len, long timeout)
{
    char errstr[512];
    rd_kafka_topic_t *rkt = NULL;
    int partition = RD_KAFKA_PARTITION_UA;
    rd_kafka_topic_conf_t *conf = NULL;
    struct produce_cb_params pcb = {1, 0, 0, 0, 0, NULL};

    if (r == NULL)
    {
        if (log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(LOG_ERR, "No connection provided to produce to topic %s", topic);
        }
        return -2;
    }

    /* Topic configuration */
    conf = rd_kafka_topic_conf_new();

    rd_kafka_topic_conf_set(conf,"produce.offset.report", "true", errstr, sizeof errstr );

    char timeoutStr[64];
    snprintf(timeoutStr, 64, "%lu", timeout);
    if (rd_kafka_topic_conf_set(conf, "message.timeout.ms", timeoutStr, errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK)
    {
        if (log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(
                LOG_ERR,
                "Failed to configure topic param 'message.timeout.ms' to %lu before producing; config err was: %s",
                timeout,
                errstr
            );
        }
        rd_kafka_topic_conf_destroy(conf);
        return -3;
    }

    //callback already set in kafka_set_connection
    rkt = rd_kafka_topic_new(r, topic, conf);
    if (!rkt)
    {
        if (log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(LOG_ERR, "Failed to open topic %s", topic);
        }
        rd_kafka_topic_conf_destroy(conf);
        return -1;
    }

    //begin producing:
    if (rd_kafka_produce(rkt, partition, RD_KAFKA_MSG_F_COPY, msg, msg_len,NULL, 0,&pcb) == -1)
    {
        if (log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(LOG_ERR, "Failed to produce message: %s", rd_kafka_err2str(rd_kafka_errno2err(errno)));
        }
        //handle delivery response (callback)
        rd_kafka_poll(rk, 0);
        rd_kafka_topic_destroy(rkt);
        return -1;
    }
    rd_kafka_poll(rk, 0);
    while(pcb.msg_count && rd_kafka_outq_len(r) > 0)
        rd_kafka_poll(r, 10);
    rd_kafka_topic_destroy(rkt);
    return 0;
}

int kafka_produce_batch(rd_kafka_t *r, char *topic, char **msg, int *msg_len, int msg_cnt, int report, long timeout)
{
    char errstr[512];
    rd_kafka_topic_t *rkt;
    struct produce_cb_params pcb = {msg_cnt, 0, 0, 0, 0, NULL};
    void *opaque;
    int partition = RD_KAFKA_PARTITION_UA;
    int i,
        err_cnt = 0;

    if (report)
        opaque = &pcb;
    else
        opaque = NULL;
    rd_kafka_topic_conf_t *topic_conf;

    if (r == NULL)
    {
        if (log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(LOG_ERR, "phpkafka - no connection to produce to topic: %s", topic);
        }
        return -2;
    }

    /* Topic configuration */
    topic_conf = rd_kafka_topic_conf_new();

    char timeoutStr[64];
    snprintf(timeoutStr, 64, "%lu", timeout);
    if (rd_kafka_topic_conf_set(topic_conf, "message.timeout.ms", timeoutStr, errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK)
    {
        if (log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(
                LOG_ERR,
                "Failed to configure topic param 'message.timeout.ms' to %lu before producing; config err was: %s",
                timeout,
                errstr
            );
        }
        rd_kafka_topic_conf_destroy(topic_conf);
        return -3;
    }

    /* Create topic */
    rkt = rd_kafka_topic_new(r, topic, topic_conf);

    //do we have VLA?
    rd_kafka_message_t *messages = calloc(sizeof *messages, msg_cnt);
    if (messages == NULL)
    {//fallback to individual produce calls
        for (i=0;i<msg_cnt;++i)
        {
            if (rd_kafka_produce(rkt, partition, RD_KAFKA_MSG_F_COPY, msg[i], msg_len[i], NULL, 0, opaque) == -1)
            {
                if (log_level)
                {
                    openlog("phpkafka", 0, LOG_USER);
                    syslog(LOG_INFO, "phpkafka - %% Failed to produce to topic %s "
                        "partition %i: %s",
                        rd_kafka_topic_name(rkt), partition,
                        rd_kafka_err2str(
                        rd_kafka_errno2err(errno)));
                }
            }
        }
    }
    else
    {
        for (i=0;i<msg_cnt;++i)
        {
            messages[i].payload = msg[i];
            messages[i].len = msg_len[i];
        }
        i = rd_kafka_produce_batch(rkt, partition, RD_KAFKA_MSG_F_COPY, messages, msg_cnt);
        if (i < msg_cnt)
        {
            if (log_level)
            {
                openlog("phpkafka", 0, LOG_USER);
                syslog(LOG_WARNING, "Failed to queue full message batch, %d of %d were put in queue", i, msg_cnt);
            }
        }
        err_cnt = msg_cnt - i;
        free(messages);
        messages = NULL;
    }
    /* Poll to handle delivery reports */
    rd_kafka_poll(r, 0);

    /* Wait for messages to be delivered */
    while (report && pcb.msg_count && rd_kafka_outq_len(r) > 0)
        rd_kafka_poll(r, 10);

    //set global to NULL again
    rd_kafka_topic_destroy(rkt);
    if (report)
        err_cnt = pcb.err_count;
    return err_cnt;
}

int kafka_produce(rd_kafka_t *r, char* topic, char* msg, int msg_len, int report, long timeout)
{

    char errstr[512];
    rd_kafka_topic_t *rkt;
    struct produce_cb_params pcb = {1, 0, 0, 0, 0, NULL};
    void *opaque;
    int partition = RD_KAFKA_PARTITION_UA;

    //decide whether to pass callback params or not...
    if (report)
        opaque = &pcb;
    else
        opaque = NULL;

    rd_kafka_topic_conf_t *topic_conf;

    if (r == NULL)
    {
        if (log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(LOG_ERR, "phpkafka - no connection to produce to topic: %s", topic);
        }
        return -2;
    }

    /* Topic configuration */
    topic_conf = rd_kafka_topic_conf_new();

    char timeoutStr[64];
    snprintf(timeoutStr, 64, "%lu", timeout);
    if (rd_kafka_topic_conf_set(topic_conf, "message.timeout.ms", timeoutStr, errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK)
    {
        if (log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(
                LOG_ERR,
                "Failed to configure topic param 'message.timeout.ms' to %lu before producing; config err was: %s",
                timeout,
                errstr
            );
        }
        rd_kafka_topic_conf_destroy(topic_conf);
        return -3;
    }

    /* Create topic */
    rkt = rd_kafka_topic_new(r, topic, topic_conf);

    if (rd_kafka_produce(rkt, partition,
                     RD_KAFKA_MSG_F_COPY,
                     /* Payload and length */
                     msg, msg_len,
                     /* Optional key and its length */
                     NULL, 0,
                     /* Message opaque, provided in
                      * delivery report callback as
                      * msg_opaque. */
                     opaque) == -1) {
       if (log_level) {
           openlog("phpkafka", 0, LOG_USER);
           syslog(LOG_INFO, "phpkafka - %% Failed to produce to topic %s "
               "partition %i: %s",
               rd_kafka_topic_name(rkt), partition,
               rd_kafka_err2str(
               rd_kafka_errno2err(errno)));
        }
       rd_kafka_topic_destroy(rkt);
       return -1;
    }

    /* Poll to handle delivery reports */
    rd_kafka_poll(r, 0);

    /* Wait for messages to be delivered */
    while (report && pcb.msg_count && rd_kafka_outq_len(r) > 0)
      rd_kafka_poll(r, 10);

    //set global to NULL again
    rd_kafka_topic_destroy(rkt);
    return 0;
}

static void offset_queue_consume(rd_kafka_message_t *message, void *opaque)
{
    struct consume_cb_params *params = opaque;
    if (params->eop == 0)
        return;
    if (message->err)
    {
        params->error_count += 1;
        if (params->auto_commit == 0)
            rd_kafka_offset_store(
                message->rkt,
                message->partition,
                message->offset == 0 ? 0 : message->offset -1
            );
        if (message->err == RD_KAFKA_RESP_ERR__PARTITION_EOF)
        {
            if (params->partition_offset[message->partition] == -2)
            {//no previous message read from this partition
             //set offset value to last possible value (-1 or last existing)
             //reduce eop count
                params->eop -= 1;
                params->read_count += 1;
                params->partition_offset[message->partition] = message->offset -1;
            }
            if (log_level)
            {
                openlog("phpkafka", 0, LOG_USER);
                syslog(LOG_INFO,
                    "phpkafka - %% Consumer reached end of %s [%"PRId32"] "
                    "message queue at offset %"PRId64"\n",
                    rd_kafka_topic_name(message->rkt),
                    message->partition, message->offset);
            }
        }
        return;
    }
    if (params->partition_offset[message->partition] == -1)
        params->eop -= 1;
    //we have an offset, save it
    params->partition_offset[message->partition] = message->offset;
    //tally read_count
    params->read_count += 1;
    if (params->auto_commit == 0)
        rd_kafka_offset_store(
            message->rkt,
            message->partition,
            message->offset == 0 ? 0 : message->offset -1
        );
}

static void queue_consume(rd_kafka_message_t *message, void *opaque)
{
    struct consume_cb_params *params = opaque;
    zval *return_value = params->return_value;

    //all partitions EOF
    if (params->eop < 1)
        return;
    //nothing more to read...
    if (params->read_count == 0)
        return;

	php_printf("========auto_commit:%d\n",params->auto_commit);

    if (message->err)
    {
        params->error_count += 1;
        //if auto-commit is disabled:
        if (params->auto_commit == 0)
            //store offset
            rd_kafka_offset_store(
                message->rkt,
                message->partition,
                message->offset == 0 ? 0 : message->offset -1
            );
        if (message->err == RD_KAFKA_RESP_ERR__PARTITION_EOF)
        {
            if (params->partition_ends[message->partition] == 0)
            {
                params->eop -= 1;
                params->partition_ends[message->partition] = 1;
            }
            if (log_level)
            {
                openlog("phpkafka", 0, LOG_USER);
                syslog(LOG_INFO,
                    "phpkafka - %% Consumer reached end of %s [%"PRId32"] "
                    "message queue at offset %"PRId64"\n",
                    rd_kafka_topic_name(message->rkt),
                    message->partition, message->offset);
            }
            return;
        }
        //add_next_index_string(return_value, rd_kafka_message_errstr(message), 1);
        if (log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(LOG_INFO, "phpkafka - %% Consume error for topic \"%s\" [%"PRId32"] "
                "offset %"PRId64": %s\n",
                rd_kafka_topic_name(message->rkt),
                message->partition,
                message->offset,
                rd_kafka_message_errstr(message)
            );
        }
        return;
    }
    //only count successful reads!
    //-1 means read all from offset until end
    if (params->read_count != -1)
        params->read_count -= 1;
    //add message to return value (perhaps add as array -> offset + msg?
    if (message->len > 0) {
        add_next_index_stringl(
            return_value,
            (char *) message->payload,
            (int) message->len,
            1
        );
    } else {
        add_next_index_string(return_value, "", 1);
    }

    //store offset if autocommit is disabled
    if (params->auto_commit == 0)
        rd_kafka_offset_store(
            message->rkt,
            message->partition,
            message->offset
        );
}

static rd_kafka_message_t *msg_consume(rd_kafka_message_t *rkmessage,
       void *opaque)
{
    int *run = opaque;
    if (rkmessage->err)
    {
        *run = 0;
        if (rkmessage->err == RD_KAFKA_RESP_ERR__PARTITION_EOF)
        {
            if (log_level)
            {
                openlog("phpkafka", 0, LOG_USER);
                syslog(LOG_INFO,
                    "phpkafka - %% Consumer reached end of %s [%"PRId32"] "
                    "message queue at offset %"PRId64"\n",
                    rd_kafka_topic_name(rkmessage->rkt),
                    rkmessage->partition, rkmessage->offset);
            }
            return NULL;
        }
        if (log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(LOG_INFO, "phpkafka - %% Consume error for topic \"%s\" [%"PRId32"] "
                "offset %"PRId64": %s\n",
                rd_kafka_topic_name(rkmessage->rkt),
                rkmessage->partition,
                rkmessage->offset,
                rd_kafka_message_errstr(rkmessage)
            );
        }
        return NULL;
    }

    return rkmessage;
}

//get topics + partition count
void kafka_get_topics(rd_kafka_t *r, zval *return_value)
{
    int i;
    const struct rd_kafka_metadata *meta = NULL;
    if (r == NULL)
    {
        if (log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(LOG_ERR, "phpkafka - no connection to get topics");
        }
        return;
    }
    if (RD_KAFKA_RESP_ERR_NO_ERROR == rd_kafka_metadata(r, 1, NULL, &meta, 200)) {
        for (i=0;i<meta->topic_cnt;++i) {
            add_assoc_long(
               return_value,
               meta->topics[i].topic,
               (long) meta->topics[i].partition_cnt
            );
        }
    }
    if (meta) {
        rd_kafka_metadata_destroy(meta);
    }
}

static
int kafka_partition_count(rd_kafka_t *r, const char *topic)
{
    rd_kafka_topic_t *rkt;
    rd_kafka_topic_conf_t *conf;
    int i;//C89 compliant
    //connect as consumer if required
    if (r == NULL)
    {
        if (log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(LOG_ERR, "phpkafka - no connection to get partition count for topic: %s", topic);
        }
        return -1;
    }
    /* Topic configuration */
    conf = rd_kafka_topic_conf_new();

    /* Create topic */
    rkt = rd_kafka_topic_new(r, topic, conf);
    //metadata API required rd_kafka_metadata_t** to be passed
    const struct rd_kafka_metadata *meta = NULL;
    if (RD_KAFKA_RESP_ERR_NO_ERROR == rd_kafka_metadata(r, 0, rkt, &meta, 200))
        i = (int) meta->topics->partition_cnt;
    else
        i = 0;
    if (meta) {
        rd_kafka_metadata_destroy(meta);
    }
    rd_kafka_topic_destroy(rkt);
    return i;
}

//get the available partitions for a given topic
void kafka_get_partitions(rd_kafka_t *r, zval *return_value, char *topic)
{
    //we need a connection!
    if (r == NULL)
        return;
    int i, count = kafka_partition_count(r, topic);
    for (i=0;i<count;++i) {
        add_next_index_long(return_value, i);
    }
}

/**
 * @brief Get all partitions for topic and their beginning offsets, useful
 * if we're consuming messages without knowing the actual partition beforehand
 * @param int **partitions should be pointer to NULL, will be allocated here
 * @param const char * topic topic name
 * @return int (0 == meta error, -2: no connection, -1: allocation error, all others indicate success (nr of elems in array))
 */
int kafka_partition_offsets(rd_kafka_t *r, long **partitions, const char *topic)
{
    rd_kafka_topic_t *rkt = NULL;
    rd_kafka_topic_conf_t *conf = NULL;
    rd_kafka_queue_t *rkqu = NULL;
    struct consume_cb_params cb_params = {0, NULL, NULL, 0, 0, 0};
    int i = 0;
    //make life easier, 1 level of indirection...
    long *values = *partitions;
    //connect as consumer if required
    if (r == NULL)
    {
        if (log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(LOG_ERR, "phpkafka - no connection to get offsets of topic: %s", topic);
        }
        return -2;
    }
    /* Topic configuration */
    conf = rd_kafka_topic_conf_new();

    /* Create topic */
    rkt = rd_kafka_topic_new(r, topic, conf);
    rkqu = rd_kafka_queue_new(rk);
    const struct rd_kafka_metadata *meta = NULL;
    if (RD_KAFKA_RESP_ERR_NO_ERROR == rd_kafka_metadata(r, 0, rkt, &meta, 5))
    {
        values = realloc(values, meta->topics->partition_cnt * sizeof *values);
        if (values == NULL)
        {
            *partitions = values;//possible corrupted pointer now
            //free metadata, return error
            rd_kafka_metadata_destroy(meta);
            return -1;
        }
        //we need eop to reach 0, if there are 4 partitions, start at 3 (0, 1, 2, 3)
        cb_params.eop = meta->topics->partition_cnt -1;
        cb_params.partition_offset = values;
        for (i=0;i<meta->topics->partition_cnt;++i)
        {
            //initialize: set to -2 for callback
            values[i] = -2;
            if (rd_kafka_consume_start_queue(rkt, meta->topics->partitions[i].id, RD_KAFKA_OFFSET_BEGINNING, rkqu))
            {
                if (log_level)
                {
                    openlog("phpkafka", 0, LOG_USER);
                    syslog(LOG_ERR,
                        "Failed to start consuming topic %s [%"PRId32"]",
                        topic, meta->topics->partitions[i].id
                    );
                }
                continue;
            }
        }
        //eiter eop reached 0, or the read errors >= nr of partitions
        //either way, we've consumed a message from each partition, and therefore, we're done
        while(cb_params.eop && cb_params.error_count < meta->topics->partition_cnt)
            rd_kafka_consume_callback_queue(rkqu, 100, offset_queue_consume, &cb_params);
        //stop consuming for all partitions
        for (i=0;i<meta->topics->partition_cnt;++i)
            rd_kafka_consume_stop(rkt, meta->topics[0].partitions[i].id);
        rd_kafka_queue_destroy(rkqu);
        //do we need this poll here?
        while(rd_kafka_outq_len(r) > 0)
            rd_kafka_poll(r, 5);

        //let's be sure to pass along the correct values here...
        *partitions = values;
        i = meta->topics->partition_cnt;
    }
    if (meta)
        rd_kafka_metadata_destroy(meta);
    rd_kafka_topic_destroy(rkt);
    return i;
}

void kafka_consume_all(rd_kafka_t *rk, zval *return_value, const char *topic, const char *offset, int item_count)
{
    char errstr[512];
    rd_kafka_topic_t *rkt;
    rd_kafka_topic_conf_t *conf;
    const struct rd_kafka_metadata *meta = NULL;
    rd_kafka_queue_t *rkqu = NULL;
    int current, p, i = 0;
    int32_t partition = 0;
    int64_t start;
    struct consume_cb_params cb_params = {item_count, return_value, NULL, 0, 0, 0};
    //check for NULL pointers, all arguments are required!
    if (rk == NULL || return_value == NULL || topic == NULL || offset == NULL || strlen(offset) == 0)
        return;

    if (!strcmp(offset, "end"))
        start = RD_KAFKA_OFFSET_END;
    else if (!strcmp(offset, "beginning"))
        start = RD_KAFKA_OFFSET_BEGINNING;
    else if (!strcmp(offset, "stored"))
        start = RD_KAFKA_OFFSET_STORED;
    else
        start = strtoll(offset, NULL, 10);

    /* Topic configuration */
    conf = rd_kafka_topic_conf_new();

    /* Disable autocommit, queue_consume sets offsets automatically */
    if (rd_kafka_topic_conf_set(conf, "auto.commit.enable", "false", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK)
    {
        if (log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(
                LOG_WARNING,
                "failed to turn autocommit off consuming %d messages (start offset %"PRId64") from topic %s: %s",
                item_count,
                start,
                topic,
                errstr
            );
        }
        cb_params.auto_commit = 1;
    }
    /* Create topic */
    rkt = rd_kafka_topic_new(rk, topic, conf);
    if (!rkt)
    {
        if (log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(LOG_INFO, "phpkafka - Failed to read %s from %"PRId64" (%s)", topic, start, offset);
        }
        return;
    }
    rkqu = rd_kafka_queue_new(rk);
    if (RD_KAFKA_RESP_ERR_NO_ERROR == rd_kafka_metadata(rk, 0, rkt, &meta, 5))
    {
        p = meta->topics->partition_cnt;
        cb_params.partition_ends = calloc(sizeof *cb_params.partition_ends, p);
        if (cb_params.partition_ends == NULL)
        {
            if (log_level)
            {
                openlog("phpkafka", 0, LOG_USER);
                syslog(LOG_INFO, "phpkafka - Failed to read %s from %"PRId64" (%s)", topic, start, offset);
            }
            rd_kafka_metadata_destroy(meta);
            meta = NULL;
            rd_kafka_queue_destroy(rkqu);
            rd_kafka_topic_destroy(rkt);
            return;
        }
        cb_params.eop = p;
        for (i=0;i<p;++i)
        {
            partition = meta->topics[0].partitions[i].id;
            if (rd_kafka_consume_start_queue(rkt, partition, start, rkqu))
            {
                if (log_level)
                {
                    openlog("phpkafka", 0, LOG_USER);
                    syslog(LOG_ERR,
                        "Failed to start consuming topic %s [%"PRId32"]: %s",
                        topic, partition, offset
                    );
                }
                continue;
            }
        }
        while(cb_params.read_count && cb_params.eop)
            rd_kafka_consume_callback_queue(rkqu, 200, queue_consume, &cb_params);
        free(cb_params.partition_ends);
        cb_params.partition_ends = NULL;
        for (i=0;i<p;++i)
        {
            partition = meta->topics[0].partitions[i].id;
            rd_kafka_consume_stop(rkt, partition);
        }
        rd_kafka_metadata_destroy(meta);
        meta = NULL;
        rd_kafka_queue_destroy(rkqu);
        while(rd_kafka_outq_len(rk) > 0)
            rd_kafka_poll(rk, 50);
        rd_kafka_topic_destroy(rkt);
    }
    if (meta)
        rd_kafka_metadata_destroy(meta);
}

int kafka_consume(rd_kafka_t *r, zval* return_value, char* topic, char* offset, int item_count, int partition)
{
    int64_t start_offset = 0;
    int read_counter = 0,
        run = 1;
    //nothing to consume?
    if (item_count == 0)
        return 0;
    if (strlen(offset) != 0)
    {
        if (!strcmp(offset, "end"))
            start_offset = RD_KAFKA_OFFSET_END;
        else if (!strcmp(offset, "beginning"))
            start_offset = RD_KAFKA_OFFSET_BEGINNING;
        else if (!strcmp(offset, "stored"))
            start_offset = RD_KAFKA_OFFSET_STORED;
        else
        {
            start_offset = strtoll(offset, NULL, 10);
            if (start_offset < 1)
                return -1;
        }

    }
    rd_kafka_topic_t *rkt;

    if (r == NULL)
    {
        if (log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(LOG_ERR, "phpkafka - no connection to consume from topic: %s", topic);
        }
        return -2;
    }

    rd_kafka_topic_conf_t *topic_conf;

    /* Topic configuration */
    topic_conf = rd_kafka_topic_conf_new();

    /* Create topic */
    rkt = rd_kafka_topic_new(r, topic, topic_conf);
    if (rkt == NULL)
    {
        if (log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(
                LOG_ERR,
               "Failed to consume from topic %s: %s",
                topic,
                rd_kafka_err2str(
                    rd_kafka_errno2err(errno)
                )
            );
        }
        return -3;
    }
    if (log_level)
    {
        openlog("phpkafka", 0, LOG_USER);
        syslog(LOG_INFO, "phpkafka - start_offset: %"PRId64" and offset passed: %s", start_offset, offset);

    }
    /* Start consuming */
    if (rd_kafka_consume_start(rkt, partition, start_offset) == -1)
    {
        if (log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(LOG_INFO, "phpkafka - %% Failed to start consuming: %s",
                rd_kafka_err2str(rd_kafka_errno2err(errno)));
        }
        return -4;
    }

    /**
     * Keep reading until run == 0, or read_counter == item_count
     */
    for (read_counter=0;read_counter!=item_count;++read_counter)
    {
        if (run == 0)
            break;
        if (log_level)
        {
            openlog("phpkafka", 0, LOG_USER);
            syslog(LOG_INFO, "Consuming, count at %d (of %d - run: %d)",
                read_counter,
                item_count,
                run
            );
        }
        rd_kafka_message_t *rkmessage = NULL,
            *rkmessage_return = NULL;

        /* Consume single message.
         * See rdkafka_performance.c for high speed
         * consuming of messages. */
        rkmessage = rd_kafka_consume(rkt, partition, 1000);
        //timeout ONLY if error didn't cause run to be 0
        if (!rkmessage)
        {
            //break on timeout, makes second call redundant
            if (errno == ETIMEDOUT)
            {
                if (log_level)
                {
                    openlog("phpkafka", 0, LOG_USER);
                    syslog(LOG_INFO, "Consumer timed out, count at %d (of %d) stop consuming after %d messages",
                        read_counter,
                        item_count,
                        read_counter +1
                    );
                }
                break;
            }
            continue;
        }

        rkmessage_return = msg_consume(rkmessage, &run);
        if (rkmessage_return != NULL)
        {
            if ((int) rkmessage_return->len > 0)
            {
                add_index_stringl(
                    return_value,
                    (int) rkmessage_return->offset,
                    (char *) rkmessage_return->payload,
                    (int) rkmessage_return->len,
                    1
                );
            }
            else
            {
                add_index_string(return_value, (int) rkmessage_return->offset, "", 1);
            }
        }
        /* Return message to rdkafka */
        rd_kafka_message_destroy(rkmessage);
    }

    /* Stop consuming */
    rd_kafka_consume_stop(rkt, partition);
    rd_kafka_topic_destroy(rkt);
    return 0;
}

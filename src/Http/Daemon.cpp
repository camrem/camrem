//
// Created by Christopher Schmidt on 30.07.15.
//

#include "Daemon.h"
#include "Http.h"

Http::Daemon::Daemon() {
    daemon_ = NULL;
}

Http::Daemon::Daemon(json config) {
    daemon_ = NULL;

    this->init(config);
    this->run();
}

Http::Daemon::~Daemon() {
    MHD_stop_daemon(daemon_);
}

bool Http::Daemon::init(json config) {
    // Check if the port is a valid number
    if ( not config["port"].is_number_integer() ) {
        BOOST_LOG_TRIVIAL(fatal) << "Invalid HTTP config: Port is not set or NaN";
        return false;
    }

    // Store port value for convenience
    port_ = config["port"];

    // Check if the port is in range
    if ( port_ < MIN_PORT || port_ > MAX_PORT ) {
        BOOST_LOG_TRIVIAL(fatal) << "Invalid HTTP config: Port must be between "
                                 << MIN_PORT << " and " << MAX_PORT
                                 << ", but is " << port_;
        return false;
    }

    // Check if SSL is enabled, if not we're done here.
    if ( config["ssl"].is_boolean() && not config["ssl"] ) {
        return true;
    }

    // Check if the SSL config appears to be valid
    if ( not config["ssl"].is_object() ) {
        BOOST_LOG_TRIVIAL(fatal) << "Invalid SSL config.";
        return false;
    }

    if ( not config["ssl"]["cert"].is_string() || not config["ssl"]["key"].is_string() ) {
        BOOST_LOG_TRIVIAL(fatal) << "Invalid SSL config.";
        return false;
    }

    // Load SSL key and certificate
    key_ = load_file(config["ssl"]["key"]);
    cert_ = load_file(config["ssl"]["cert"]);

    // Check if the files are loaded
    if ( key_ == NULL || cert_ == NULL ) {
        BOOST_LOG_TRIVIAL(fatal) << "Error loading keyfiles";
        return false;
    }

    return true;
}

int Http::Daemon::handle_connection(void *cls, struct MHD_Connection *connection, const char *uri, const char *method,
                                  const char *version, const char *upload_data, size_t *upload_data_size,
                                  void **con_cls) {
    struct MHD_Response *mhd_response;
    int ret;
    unsigned int status;
    Request *request;
    shared_ptr<Response> response;

    // Check if the request is created already, or create it.
    if ( NULL == *con_cls ) {
        request = new Request(connection, uri, method);
        *con_cls = (void *) request;
        return MHD_YES;
    } else {
        request = (Request *) *con_cls;
    }

    // Process POST/PUT data
    // TODO: Check for max_upload_size
    if ( *upload_data_size > 0 ) {
        request->postdata_.append(upload_data);
        *upload_data_size = 0;

        return MHD_YES;
    }

    // Process the request in Http main class
    if ( Http::getInstance()->processRequest(request) ) {
        response = request->response;
        status = response->status;

        mhd_response = MHD_create_response_from_buffer(response->getContent()->length(),
                                                       (void*) response->getContent()->c_str(),
                                                       MHD_RESPMEM_PERSISTENT);
    } else {
        const char *error = "Internal Server Error";
        status = MHD_HTTP_INTERNAL_SERVER_ERROR;

        mhd_response = MHD_create_response_from_buffer(strlen(error), (void*) error, MHD_RESPMEM_PERSISTENT);
    }

    // Enqueue response and free resources
    ret = MHD_queue_response(connection, status, mhd_response);
    MHD_destroy_response(mhd_response);

    return ret;
}

bool Http::Daemon::run() {
    // Check for already running daemons
    if ( NULL != daemon_ ) {
        return false;
    }

    daemon_ = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, port_, NULL, NULL,
                               &handle_connection, NULL,
                               MHD_OPTION_NOTIFY_COMPLETED, &Request::completed, NULL,
                               MHD_OPTION_END);

    if ( NULL == daemon_ ) {
        BOOST_LOG_TRIVIAL(warning) << "Could not bind http daemon to port " << port_;
        return false;
    }

    BOOST_LOG_TRIVIAL(info) << "Listening on port " << port_;
    return true;
}


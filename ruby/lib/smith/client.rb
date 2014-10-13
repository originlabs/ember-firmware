require 'smith'
require 'smith/client/url_helper'
require 'logger'
require 'faye'

Dir["#{Smith.root}/lib/smith/client/**/*.rb"].each { |f| require(f) }

module Smith
  module Client

    class << self
      attr_accessor :logger
    end

    module_function

    def log_info(message)
      logger.info(message) if logger
    end

    def log_error(message)
      logger.error(message) if logger
    end

    def log_debug(message)
      logger.debug(message) if logger
    end

    def log_warn(message)
      logger.warn(message) if logger
    end

    def enable_logging(logdev = nil, formatter = nil)
      # Flush stdout immediately if using STDOUT as logdev
      STDOUT.sync = true if logdev.nil?
      self.logger = Logger.new(logdev || STDOUT)
      logger.progname = 'smith-client'
      logger.formatter = formatter if formatter
    end

    def enable_faye_logging(logdev = nil)
      # Flush stdout immediately if using STDOUT as logdev
      STDOUT.sync = true if logdev.nil?
      Faye.logger = Logger.new(logdev || STDOUT)
      Faye.logger.level = Logger::INFO
    end

    def start
      EventLoop.new(State.load, 60).start
    end

  end
end
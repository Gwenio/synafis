#!/usr/bin/env ruby

# Copyright (c) 2014 James Adam Armstrong

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

#----------------------

# A class to manage output of Ninja files.
class NinjaGen

  private

  @id_filter = /\A([a-z_]+)\Z/

  class << self
    attr_accessor :id_filter
  end

  attr_accessor :file

  # Tests if text is a valid id using the id_filter regex.
  def valid_id(text)
	  NinjaGen.id_filter.match(text)
  end

  # Outputs a list of variables with proper indentation for builds and rules.
  def var_set(vars)
    vars.each_pair do |var, value|
      @file.write("  ")
      variable(var, value)
    end
    @file.write("\n") unless vars.empty?
  end

  public

  # Sets up the instance with an open file.
  def initialize(file)
    @file = file
  end

  # Outputs a comment.
  def comment(text)
    @file.write("\# #{text}\n")
    self
  end

  # Outputs an include directive.
  def include(name)
    @file.write("include #{name}\n")
    self
  end

  # Outputs an subninja directive.
  def subninja(name)
    @file.write("subninja #{name}\n")
    self
  end

  # Outputs a variable statement.
  def variable(name, value)
    raise "Invalid variable name '#{name}'." unless valid_id(name)
    @file.write("#{name} = #{value}\n")
    self
  end

  # Outputs a pool statement.
  def pool(name, value)
    raise "Invalid pool name '#{name}'." unless valid_id(name)
    @file.write("pool #{name}\n  depth = #{value}\n")
    self
  end

  # Outputs a build description.
  def build(outputs, rule, inputs, vars)
    raise "Invalid build name '#{rule}'." unless valid_id(rule)
    @file.write("build #{outputs.join(' ')}: #{rule} #{inputs.join(' ')}\n")
    var_set(vars)
    newline()
  end

  # Outputs a ninja rule. First parameter is the name, second is a map of variables and their values.
  def command(rule, vars)
    raise "Invalid rule name '#{rule}'." unless valid_id(rule)
    @file.write("rule #{rule}\n")
    var_set(vars)
    self
  end

  # Outputs a new line.
  def default_targets(products)
    @file.write("default #{products.join(' ')}\n")
    self
  end

  # Outputs a new line.
  def newline()
    @file.write("\n")
    self
  end

  # Closes the internal file object.
  def close()
    @file.close
  end

  def self.open(filename, opt, block = Proc.new)
    File.open(filename, opt) { |file| block.call(NinjaGen.new(file)) }
  end

end

#----------------------

# Class for storing information for file groups.
class FileIndex
  private

  # Stores the file index.
  attr_accessor :store

  # Stores the regex index.
  attr_accessor :regex

  # Stores the patterns index.
  attr_accessor :patterns

  public

  # Sets up the instance with project data.
  def initialize(project)
    @store = {}
    @regex = project['regex'] || {}
    @patterns = project['patterns'] || {}

    @regex.each_pair { |key, val| @regex[key] = Regexp.new(val) }

    files = project['files']
    raise "Project file list not found in 'project.json'." unless files

    files.each_pair do |name, group|
      @store[name] = collect(group)
    end
  end

  # Retrieves a file group.
  def at(key)
    @store[key]
  end

  # Append files to a file group or create it if needed.
  def append(key, files)
    raise "File group names must be a string." unless key.is_a?(String)
    temp = @store[key]
    @store[key] = if temp
      temp + files
    else
      files
    end
  end

  # Collects a list of files from a specification.
  def collect(spec)
    raise "Build group specificaitons are expected to be a hash." unless spec.is_a?(Hash)
    list = spec['list'] || []
    others = spec['include']
    others.each { |other| list.concat(@store[other]) } if others
    glob = spec['glob']
    if glob
      glob.each do |g|
        Dir.glob(g) do |file|
          list.push(file)
        end
      end
    end
    capture = @regex[spec['regex']]
    if capture
      list = derive(list, capture, spec['pattern'])
    else
      list.uniq!
    end
    blacklist = spec['exclude']
    if blacklist
      (list - collect(blacklist))
    else
      list
    end
  end

  # Derives a new list of files from an old one.
  def derive(list, capture, pattern)
    return [] if list.nil? || pattern.nil?
    inputs = []
    capture = @regex[capture]
    if pattern
      temp = @patterns[pattern]
      pattern = temp['pattern']
      replace = temp['replace']
      list.each do |file|
        value = String.new(pattern)
        value[replace] = file[capture]
        inputs.push(value)
      end
    else
      list.each { |file| inputs.push(file[capture]) }
    end
    inputs.uniq
  end

  def self.open(filename, opt, block = Proc.new)
    File.open(filename, opt) { |file| block.call(NinjaGen.new(file)) }
  end
end

#----------------------

# Project generation program.

require 'optparse'
require 'yajl'

help = false
builds = false
settings = false

commands = OptionParser.new do |opts|
  opts.banner = "Usage: ninjagen.rb [options]"
  opts.separator ""
  opts.separator "Creates."
  opts.separator ""
  opts.separator "Specific options:"

  opts.on("--project", "Generates '.ninja' files from 'project.json' and 'config.json'.") { || builds = true }

  opts.on("--config", "Generates a default 'config.json'.") { || settings = true }

  opts.on("--help", "Displays this help message.") { || help = true }
end

project = {}
config = {}

if !ARGV.empty?
  commands.parse(ARGV)

  if settings || builds
    loader = File.new('project.json')
    project = Yajl::Parser.parse(loader)
    loader.close
  end
else
  help = true
end

if help
  puts commands.help
end

if settings
  output = File.open("config.json", 'w')
  Yajl::Encoder.encode(config, output)
  output.close
elsif builds
  loader = File.new('config.json')
  config = Yajl::Parser.parse(loader)
  loader.close
end

if builds
  file_index = FileIndex.new(project)

  output = Hash.new do |hash, key|
    file = NinjaGen.new(File.new("#{key}.ninja", 'w+b'))

    file.newline

    file.comment("This file is generated from 'project.json' by 'ninjagen.rb'.\n")

    hash[key] = file
  end

  spec = project['build']
  raise "Project build list not found in 'project.json'." unless spec

  spec.each do |group|
    rule = group['rule']
    vars = group['vars'] || {}
    filename = group['file']
    raise "Output file for a build rule not found in 'project.json'." unless filename
    file = output[filename]
    infoproc = Proc.new do |key|
      info = group[key]
      if info
        file_index.collect(info)
      else
        []
      end
    end
    inputs = infoproc.call('input')
    outputs = infoproc.call('output')
    imp_inputs = infoproc.call('imp_input')
    imp_outputs = infoproc.call('imp_output')
    order = infoproc.call('order')
    index = group['index']
    file_index.append(index, outputs) if index
    imp_inputs.unshift('|') if !imp_inputs.empty?
    imp_inputs += ['||'] + order unless order.empty?
    outputs += ['|'] + imp_outputs unless imp_outputs.empty?
    regex = group['regex']
    if regex
      derived = file_index.derive(inputs, regex, group['pattern'])
      file_index.append(index, derived) if index
      sources = inputs.each
      imp_inputs.unshift('')
      outputs.unshift('')
      derived.each do |product|
        outputs[0] = product
        imp_inputs[0] = sources.next
        file.build(outputs, rule, imp_inputs, vars)
      end
    else
      inputs += imp_inputs unless imp_inputs.empty?
      file.build(outputs, rule, inputs, vars)
    end
  end

  output.each_pair { |key, file| file.close() }

  tools = project['tools']
  raise "No tool definitions found in 'project.json'." unless tools

  chains = config['toolchains']
  raise "Expected at least one toolchain in 'config.json'." unless chains

  chains.each do |value|
    name = value['name']
    raise "No name listed for a toolchain entry in 'config.json'." unless name

    NinjaGen.open("#{name}.ninja", 'w+b') do |output|

      output.newline.comment("This file is generated from 'project.json' and 'config.json' by 'ninjagen.rb'.\n")

      files = []

      tools.each_pair do |key, info|
        select = value[key]
        select = info['default'] unless select
        if select
          rule = info['options'][select]
          raise "Tool specified for '${key} in 'config.json' not found in 'project.json'." unless rule
          desc = info['description']
          output.comment(desc) if desc
          rule['description'] = select
          output.command(key, rule)
          files.push(info['file'])
        else
          raise "No tool selected for rule type '${key}' in 'config.json'." if info['optional'] != true
        end
      end

      files.uniq.each { |file| output.include("#{file}.ninja") }
    end
  end

  targets = config['targets']
  raise "Expected at least one build target in 'config.json'." unless targets

  vars = project['vars']
  pools = project['pools']
  ninja_version = project['ninja_version']
  builddir = config['builddir']
  subfiles = []

  buildvar = project['buildvar']
  raise "'buildvar' not specified in 'project.json'." unless buildvar

  targets.each do |target|
    buildname = target['file']
    raise "A build target definition was lacking an output file in 'config.json'." unless buildname

    subfiles.push(buildname)
    NinjaGen.open("#{buildname}.ninja", 'w+b') do |output|

      output.newline.comment("This file is generated from 'config.json' and 'project.json' by 'ninjagen.rb'.\n")

      files = target['pre']
      files.each { |file| output.include(file) } if files

      output.variable(buildvar, buildname)

      target_values = target['vars']
      vars.each_pair do |var, info|
        value = target_values[var] || info['default']
        if value
          output.variable(var, value)
        else
          raise "Build target '#{buildname}' does not provide a value for variable '#{var}'." unless info['optional'] == true
        end
      end

      output.newline

      filename = target['toolchain']
      raise "A build target definition was lacking a toolchain in 'config.json'." unless filename

      output.include("#{filename}.ninja")

      files = target['post']
      files.each { |file| output.include(file) } if files
    end
  end

  NinjaGen.open("build.ninja", 'w+b') do |output|
    output.variable('ninja_required_version', ninja_version) if ninja_version
    output.variable('builddir', builddir) if builddir

    output.newline.comment("This file is generated from 'config.json' and 'project.json' by 'ninjagen.rb'.\n")

    target_pools = config['pools']
    pools.each do |pool|
      output.pool(pool, target_pools[pool] || 1)
    end

    output.newline

    subfiles.each { |file| output.subninja("#{file}.ninja") }

    files = project['defaults']
    output.default_targets(files) if files
  end

end

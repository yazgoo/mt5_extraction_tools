#!/usr/bin/env ruby
require 'pry'
require 'sinatra'
require 'builder'
$renders = "#{ENV['HOME']}/Documents/renders"
$source = "#{ENV['HOME']}/Downloads/s2/out"
$repository = "#{ENV['HOME']}/dev/mt5_extraction_tools/"
def cleanup_path r
    r.gsub($source.gsub('/', '_'), '').gsub('_', '/')
end
def png commit, id
    "#{$renders}/#{commit}/#{id}.png"
end
def mt7s
    Dir["#{$source}/**/*.MT7"].map { |a| a.gsub('/', '_') }
end
def ids
    Dir["#{$renders}/*"].map { |r| File.basename(r) }
end
def score id
    fs = mt7s
    i = 0
    missing = []
    fs.each do |f|
        if File.exist? png(id, f)
            i += 1 
        else
            missing << cleanup_path(f)
        end
    end
    {i: i, n: fs.size, missing: missing}
end
def get_previous id
    i = ids.index id
    ids[(i == 0 or i == nil) ? 0 : i - 1]
end
def describe_commit id
    Hash[{date: :cd, epoch: :ct, text: :B}.map do |x, y|
        [x, `git log --format=%#{y.to_s} -n 1 #{id} #{$repository}`]
    end]
end
get '/' do
    builder(format: :html) do |x|
        x.table do
            ids.each do |r|
                commit = describe_commit r
                p commit
                x.tr do
                    x.td commit[:date]
                    x.td { x.a r, href: "/renders/#{r}" }
                    x.td score(r)[:i]
                    x.td commit[:text]
                end
            end
        end
    end
end
get '/renders/:id' do |id|
    builder(format: :html) do |x|
        x.h1 id
        x.a "..", href: "/"
        x.a "failed list", href: "#failed"
        x.br
        commit = describe_commit id
        x.a "#{commit[:date]}, #{commit[:text]}"
        x.table do
            previous = get_previous id
            x.tr do
                x.th 'name'
                x.th id
                x.th do
                    x.a "previous (#{previous})", href: "/renders/#{previous}"
                end
            end
            x.tr do
                n = score(id)
                p = score(previous)
                x.td "score (/#{n[:n]})"
                x.td n[:i]
                x.td p[:i]
            end
            mt7s.each do |r|
                x.tr do
                    x.td cleanup_path r 
                    x.td { x.img width: '100%', src: "/render/#{id}/#{r}"}
                    x.td { x.img width: '100%', src: "/render/#{previous}/#{r}"}
                end
            end
            x.tr do
                x.td { x.a('failed list', name: 'failed') }
                x.td score(id)[:missing].join ", "
                x.td score(previous)[:missing].join ", "
            end
        end
    end
end
get '/render/:commit/:id' do |commit, id|
    png = png(commit, id)
    File.open(png) { |f| f.read } if File.exist? png
end

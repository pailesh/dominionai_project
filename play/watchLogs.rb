require 'fssm'
load("./gokoPlayer.rb")
 
LOGDIR = '/Users/oda/Library/Application Support/Google/Chrome/Default/File System/006/t/03'
BUYFEATURE_DIR = "gainFeature.txt"
PLAYFEATURE_DIR = "playFeature.txt"
ACTIONFEATURE_DIR = "actionFeature.txt"

 drawFileName = nil
 logFileName = nil

def create_action(base,file)
    puts base + "/"  + file + " was created at " + `date`
end
 
def update_action(base,file)
    puts base + "/"  + file + " was updated at " + `date`
end
 
def delete_action(base,file)
    puts base + "/" + file + " was deleted at " + `date`
end
 
FSSM.monitor(LOGDIR ,'**/*') do
 
 create do |base,file|
    if(logFileName == nil)
      logFileName = base + "/" + file
    else
      drawFileName = base + "/" + file
    end
 end
 update do |base,file|
    File.open(BUYFEATURE_DIR, 'w'){|out|
    File.open(PLAYFEATURE_DIR, 'w'){|outAction|
      File.open(ACTIONFEATURE_DIR, 'w'){|outAction2|

      #これしないと変更が反映されない
      out.sync = true
      outAction.sync = true
      outAction2.sync = true
      
      parser = GokoPlayer.new
      File.open(logFileName, 'r') {|file|
        File.open(drawFileName, 'r') {|drawfile|
          parser.parse(file, out, outAction, outAction2, drawfile, true)
        }
      }
    }
    }
  }
 end
  delete do |base,file|
     drawFileName = nil
     logFileName = nil
 end
 
end
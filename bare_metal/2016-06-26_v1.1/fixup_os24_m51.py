'''TODO: fix up os24.m51 report by applying two regex:

delete the line continuations
   replace "\n>> " with nothing

delete the page breaks
   replace "\x0c.*\n\n\n" with nothing

fix up the page breaks first, then the line continuations because sometimes the line continuations cross a page break bounardy.

'''

import re
import string

class KeilM51ReportFixer(object):
   
   def __init__(self, Filename):
      self.filename = Filename
      self.fileContentString = ""
      self.reLineContinuation = re.compile(r"\n>> ")
      self.rePageBreak = re.compile(r"\x0c.*\n\n\n")
      
      self.ReadFile(self.filename)
      self.replacePageBreaks()
      self.replaceLineContinuations()

   def ReadFile(self, Filename):
      # Read original file
      #~ print("ReadFile({0})".format(Filename))
      with open(Filename, 'r') as f:
         self.fileContentString = f.read()
      pass # implicit f.close()
      #~ print('''self.filename="{0}"'''.format(self.filename))
      #~ print('''self.fileContentString="{0}"'''.format(self.fileContentString))
      return self

   def WriteFile(self, Filename):
      #~ print("WriteFile({0})".format(Filename))
      with open(Filename, 'w') as f:
         f.write(self.fileContentString)
         #for line in lines:
         #   f.write('{0}\n'.format(line))
         #pass # end for line in lines:
      pass # implicit f.close()
      return self

   def replaceLineContinuations(self):
      #~ print("replaceLineContinuations()")
      #~ print('''self.fileContentString="{0}"'''.format(self.fileContentString))
      self.fileContentString = self.reLineContinuation.sub("", self.fileContentString)
      #~ print('''self.fileContentString="{0}"'''.format(self.fileContentString))
      return self

   def replacePageBreaks(self):
      #~ print("replacePageBreaks()")
      #~ print('''self.fileContentString="{0}"'''.format(self.fileContentString))
      self.fileContentString = self.rePageBreak.sub("", self.fileContentString)
      #~ print('''self.fileContentString="{0}"'''.format(self.fileContentString))
      return self

if __name__ == '__main__':
   print ("self-test")
   KeilM51ReportFixer(r"os24.M51").WriteFile(r"os24.M51")

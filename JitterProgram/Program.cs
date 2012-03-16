using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace CPE564JitterAnalyzer
{
    class Program
    {
        private static string workingDir = Environment.CurrentDirectory;

        static void Main(string[] args)
        {
            int timeIndex;
            string[] csvFilePaths = Directory.GetFiles(workingDir, "*.csv");
            List<double> overallJitterList = new List<double>();
            List<double> overallDelayList = new List<double>();

            foreach (string csvFilePath in csvFilePaths)
            {
                string csvFileName = Path.GetFileName(csvFilePath);
                string outFilePath = workingDir + "\\jitter_" + csvFileName;

                string[] lines = File.ReadAllLines(csvFilePath);

                string[] columnNames = lines[0].Split(',');

                for (timeIndex = 0; timeIndex < columnNames.Length && !columnNames[timeIndex].Contains("Time"); ++timeIndex)
                    ;

                double[] delays = CalculateDelays(lines, timeIndex);

                foreach (double d in delays)
                    overallDelayList.Add(d);

                double sum = 0;
                foreach (double d in delays)
                    sum += d;

                double averageDelay = sum / (double)delays.Length;

                double[] jitters = new double[delays.Length];
                for (int i = 0; i < delays.Length; ++i)
                    jitters[i] = Math.Abs(delays[i] - averageDelay);

                List<double> jitterList = jitters.ToList<double>();
                jitterList.Sort();

                WriteOutputCSVFile(outFilePath, jitterList);
            }

            double overallSum = 0;
            foreach (double d in overallDelayList)
                overallSum += d;

            double overallAverageDelay = overallSum / (double)overallDelayList.Count;

            foreach (double d in overallDelayList)
                overallJitterList.Add(Math.Abs(d - overallAverageDelay));

            overallJitterList.Sort();

            WriteOutputCSVFile(workingDir + "\\overallJitter.csv", overallJitterList);
        }

        private static double[] CalculateDelays(string[] lines, int timeIndex)
        {
            List<double> delays = new List<double>();

            for (int lineIndex = 1; lineIndex < lines.Length - 1; ++lineIndex)
            {
                double timestamp1 = GetTimestamp(lines, lineIndex, timeIndex);
                double timestamp2 = GetTimestamp(lines, lineIndex + 1, timeIndex);

                double delay = timestamp2 - timestamp1;
                delays.Add(delay);
            }

            return delays.ToArray();
        }

        private static double GetTimestamp(string[] lines, int lineIndex, int timeIndex)
        {
            string[] line = lines[lineIndex].Split(',');
            string stringTimestamp = line[timeIndex].Replace("\"", "");
            double timestamp = Convert.ToDouble(stringTimestamp);
            return timestamp;
        }

        private static void WriteOutputCSVFile(string outFilePath, List<double> sortedJitters)
        {
            //using (FileStream fs = File.Create(outFilePath))
            //{
            //    ;
            //}

            //using (StreamWriter outFileWriter = new StreamWriter(outFilePath))
            //{
            //    StringBuilder columns = new StringBuilder();
            //    columns.Append("\"Interval\",");
            //    for (int i = 1; i <= indivRunDelays.Count; ++i)
            //    {
            //        columns.Append("\"Run " + i + " Delay\",");
            //    }
            //    columns.Append("\"Average Delay\"");
            //    outFileWriter.WriteLine(columns);

            //    for (int i = 0; i < averageDelays.Length; ++i)
            //    {
            //        StringBuilder line = new StringBuilder();
            //        line.Append("\"" + (i + 1) + "\",");
            //        for (int j = 0; j < indivRunDelays.Count; ++j)
            //        {
            //            line.Append("\"" + indivRunDelays.ElementAt(j)[i] + "\",");
            //        }
            //        line.Append("\"" + averageDelays[i] + "\"");
            //        outFileWriter.WriteLine(line);
            //    }

            //    //outFileWriter.WriteLine("\"Interval\",\"Delay\"");
            //    //for (int i = 0; i < averageDelays.Length; ++i)
            //    //    outFileWriter.WriteLine("\"" + (i + 1) + "\",\"" + averageDelays[i] + "\"");
            //}

            int totalJitters = sortedJitters.Count;
            int numPreviousJitters = 0;

            using (FileStream fs = File.Create(outFilePath))
            {
                ;
            }

            using (StreamWriter outFileWriter = new StreamWriter(outFilePath))
            {
                string columns = "\"Jitter (ms)\",\"Percentile\"";
                outFileWriter.WriteLine(columns);

                foreach (double delay in sortedJitters)
                {
                    float percentile = ((float)numPreviousJitters / (float)totalJitters) * 100.0f;
                    int roundedPercentile = Convert.ToInt32(percentile);
                    string line = "\"" + (delay * 1000) + "\",\"" + roundedPercentile + "\"";
                    outFileWriter.WriteLine(line);
                    ++numPreviousJitters;
                }
            }
        }
    }
}

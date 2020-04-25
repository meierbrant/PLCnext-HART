<template>
  <b-card :class="size">
    <div class="">
        <span class="text-dark title">{{ title }}</span> <b class="value text-primary">{{ currentValue }}<span class="units text-dark">{{ units }}</span></b>
    </div>
    <line-chart :chart-data="chartData" :options="options" class="chart"></line-chart>
  </b-card>
</template>

<script lang="ts">
import { Vue, Component, Prop } from 'vue-property-decorator'

import { HartDevice, hart_device_vars_log, hart_log_var } from '../types/hart-device'
import { hartServerUrl } from '../types'
// @ts-ignore
import LineChart from './LineChart.js'
import { ChartData, ChartOptions, ChartDataSets, ChartPoint } from 'chart.js'

@Component({
    components: {
        LineChart
    }
})
export default class LineGraph extends Vue {
    @Prop() logData!: hart_log_var[]
    @Prop() title!: string
    @Prop({default: "large"}) size!: "large" | "small"
    public yStepSize?: number
    public options: ChartOptions = {
      legend: {
        display: false
      },
      maintainAspectRatio: false,
      scales: {
        yAxes: [{
          ticks: {
            maxTicksLimit: this.yTicksLimit
          }
        }],
        xAxes: [{
          ticks: {
            maxTicksLimit: this.xTicksLimit
          }
        }]
      }
    }

    mounted () {
    
    }

    get currentValue() {
      return this.logData.length > 0 ? this.logData[this.logData.length-1].value.toFixed(2) : 0
    }

    get units() {
      return this.logData.length > 0 ? this.logData[this.logData.length-1].units : ""
    }

    get xTicksLimit(): number {
      switch(this.size) {
        case "small":
          return 5
        default:
          return 10
      }
    }

    get yTicksLimit(): number {
      switch(this.size) {
        case "small":
          return 4
        default:
          return 6
      }
    }

    get chartData(): ChartData {
      let labels: string[] = []
      let chartPoints: ChartPoint[] = []
      let dataset: ChartDataSets = {
        label: this.title,
        data: [],
        pointBackgroundColor: "#0099a1",
        backgroundColor: "rgba(0,0,0,0)",
        borderColor: "#0099a1"
      }

      let start = 0
      let count = this.logData.length
      if (count > 30) {
        start = count - 31
        count = 30
      }
      for (let i=start; i<start+count; i++) {
        let v = this.logData[i]
        let date = new Date(v.timestamp)
        labels.push(date.toLocaleTimeString("en-US", {
          hour: 'numeric',
          minute: 'numeric'
        }))
        chartPoints.push({
          x: date,
          y: v.value
        })
      }
      dataset.data = chartPoints

      return {
        labels: labels,
        datasets: [dataset]
      }
    }
}
</script>

<style>
  .title {
    font-size: 1.5em;
    margin-right: 0.5em;
  }

  .value {
    font-size: 2em;
    margin-bottom: 0.5em;
  }

  .units {
    font-size: 0.7em;
  }

  .large .chart {
    max-height: 15em;
  }

  .small .chart {
    max-height: 10em;
  }
</style>

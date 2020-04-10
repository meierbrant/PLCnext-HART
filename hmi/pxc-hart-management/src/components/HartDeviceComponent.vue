<template>
    <div class="hart-device">
        <div class="row">
            <div class="col-md-6"> <!-- left column -->
                <div class="row">
                    <section class="">
                        <h2>Device Summary</h2>
                        <div class="row">
                            <div class="col-md-4 col-sm-12">
                                <img src="../assets/img/dummy-device.jpg" class="img-fluid"/>
                            </div>
                            <div class="col-md-8">
                                <table>
                                    <tr v-for="(attr, key) in hartAttrs" v-bind:key="key">
                                        <th style="text-align: right; padding-right: 0.5em;">{{ attr }}</th>
                                        <td style="text-align: left; padding-right: 0.5em;">{{ device[key] }}</td>
                                    </tr>
                                </table>
                            </div>
                        </div>
                    </section>
                </div>
            </div>
            <div class="col-md-6"> <!-- right column-->
                <section class="var-grid">
                    <h2>Vars</h2>
                    <div class="row">
                        <div class="col-sm-6" v-for="(v, key) in vars" v-bind:key="key">
                            <b-card :title="key" class="var-card">
                                <div class="value text-secondary">
                                    <!-- FIXME: if there's no var, this shouldn't get run -->
                                    {{ v.value ? v.value.toFixed(2) : '' }}<span class="units text-dark">{{ v.units ? v.units : '' }}</span>
                                </div>
                            </b-card>
                        </div>
                    </div>
                </section>
            </div>
        </div>

        <div class="row">
            <section class="">
                <div id="send-cmd">
                    <div class="input-group mb-3">
                        <!-- <input type="text" class="form-control" placeholder="command #" aria-label="cmdnum" aria-describedby="basic-addon2"> -->
                        <b-dropdown id="dropdown-1" :text="'Send a Command'" class="m-md-2">
                            <b-dropdown-item v-for="cmd in supportedCommands" v-bind:key="cmd.number" v-on:click="sendCmd(cmd);">
                                {{ cmd.number + ": " + cmd.description }}</b-dropdown-item>
                        </b-dropdown>
                        <!-- <div class="input-group-append">
                            <button class="btn btn-outline-secondary" type="button" v-on:click="sendCmd()">Send</button>
                        </div> -->
                    </div>
                    <div id="cmd-result" v-if="cmdResponse != null">
                        <h3>Response</h3>
                        <b-card no-body>
                            <b-tabs card>
                            <b-tab title="Table" active>
                                <b-table striped hover :items="cmdResponse.data"></b-table>
                            </b-tab>
                            <b-tab title="JSON">
                                <vue-json-pretty
                                    :path="'res'"
                                    :data="cmdResponse">
                                </vue-json-pretty>
                            </b-tab>
                            </b-tabs>
                        </b-card>
                    </div>
                </div>
            </section>
        </div>

        <div class="row">
            <HartVarChart :log-data="logData.pv" title="PV"></HartVarChart>
            <HartVarChart :log-data="logData.sv" title="SV"></HartVarChart>
            <HartVarChart :log-data="logData.tv" title="TV"></HartVarChart>
            <HartVarChart :log-data="logData.qv" title="QV"></HartVarChart>
        </div>
    </div>
</template>

<script lang="ts">
import { Vue, Component, Prop } from 'vue-property-decorator'
import { HartDeviceDto, verboseHartDeviceAttrs, hart_command_brief, hart_command_response, hart_device_vars_log } from '../types/hart-device'
import { HartVars } from '../types/hart-vars'
import { hartServerUrl, updateInterval } from '../types'
// @ts-ignore
import VueJsonPretty from 'vue-json-pretty'
import HartVarChart from '../components/HartVarChart.vue'
import { ChartData } from 'chart.js'

@Component({
    components: {
        VueJsonPretty,
        HartVarChart
    }
})
export default class HartDeviceComponent extends Vue {
    @Prop() device!: HartDeviceDto
    public supportedCommands: hart_command_brief[] = []
    public cmdResponse: hart_command_response | null = null
    public hartAttrs = verboseHartDeviceAttrs
    public logData: hart_device_vars_log = {
        pv: [],
        sv: [],
        tv: [],
        qv: []
    }
    private logPollInterval: number = 0

    mounted() {
        // load supported HART commands
        this.$http.get(hartServerUrl + '/gw/' + this.$route.params.serialNo + '/subdevice/' + this.device.ioCard + '/' + this.device.channel + '/commands').then(res => {
            const data = res.data as hart_command_brief[]
            this.supportedCommands = data
        })

        this.refreshChartsData()
        this.logPollInterval = setInterval(this.refreshChartsData, updateInterval)
    }

    get vars(): HartVars | undefined {
        return this.device.vars
    }

    sendCmd(cmd: hart_command_brief) {
        this.$http.get(hartServerUrl + '/gw/' + this.$route.params.serialNo + '/subdevice/' + this.device.ioCard + '/' + this.device.channel + '/cmd/' + cmd.number).then(res => {
            const data = res.data as hart_command_response
            this.cmdResponse = data
        })
    }

    private refreshChartsData() {
        this.$http.get(hartServerUrl + '/gw/' + this.$route.params.serialNo + '/log/' + this.device.ioCard + '/' + this.device.channel).then(res => {
            this.logData = res.data as hart_device_vars_log
        })
    }
}
</script>

<style scoped>
  
</style>
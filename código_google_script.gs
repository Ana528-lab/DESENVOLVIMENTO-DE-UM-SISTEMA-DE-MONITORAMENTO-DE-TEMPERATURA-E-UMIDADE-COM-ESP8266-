function doGet(e) {
// ADICIONE O ID DA SUA PLANILHA ABAIXO
  var ss = SpreadsheetApp.openById("ID_DA_SUA_PLANILHA_AQUI");
  var sheet = ss.getSheetByName("Página1");

  var data = e.parameter.data;
  var hora = e.parameter.hora;
  var temperatura = e.parameter.temperatura;
  var umidade = e.parameter.umidade;

  if (!data || !hora || !temperatura || !umidade) {
    return ContentService.createTextOutput("Erro: Parâmetros incompletos");
  }

  // Adiciona linha: Data, Hora, Temperatura, Umidade
  sheet.appendRow([data, hora, temperatura, umidade]);

  return ContentService.createTextOutput("Dados recebidos com sucesso!");
}
